#include "inirw.h"
#include"function.h"
#include "nld.h"
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <stdio.h>

//#define open(a,b) open(a,b|O_RDWR,0666)

extern int mdes3(int direction, uchar *ibuf, uchar *obuf , uchar *ikey);
extern char g_szAppPath[1024];

//CIniFile iniHead;

/****************************************************************
** �������ܣ�ͨ������nld�ļ���ȡӦ�ó����ͷ�ļ�,����ͷ�ļ������޸�ΪP����
**
** ���������
pInFilename  Ӧ�ó����ͷ�ļ�

** ���������
pHeaderInfo   	NLD����ͷ����Ϣ�ṹָ��
** ����ֵ  ��
SUCC      		�ɹ�
FAIL        	ʧ��
** ȫ�ֱ�����
** ����ģ�飺
** ˵    ������
****************************************************************/
int GetHeadInfo(const char *pInFilename,NLD_HEADERINFO *pHeaderInfo)
{
	int hFile_in;
	int nBytesRead, nBlockRead, nFileRead = 0;
	int iret;
	unsigned char unit_inbuf_tmp[UNIT_BUF_SIZE], unit_inbuf[UNIT_BUF_SIZE];
	unsigned char unit_outbuf[UNIT_BUF_SIZE];
	unsigned char des_inbuf[TDES_INPUT_SIZE], des_outbuf[TDES_INPUT_SIZE];
	unsigned char xor_inbuf_pre[TDES_INPUT_SIZE];
	unsigned char *punit_inbuf_tmp = NULL, *punit_inbuf = NULL;
	unsigned char *punit_outbuf = NULL;
	unsigned char key[16] = {1, 3, 5, 7, 2, 4, 6, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	int i, j;
	int bFileStart = TRUE, bFileEnd = FALSE;
	unsigned int iHeaderLen = 0, iNldFileLen;
	struct stat stat_buf;
	DATA1 chartoint;

	if (stat(pInFilename, &stat_buf) == FAIL) 
	{
		return FAIL;
	}
	iNldFileLen = stat_buf.st_size;

	if ((hFile_in = open(pInFilename, O_RDONLY,0666)) == FAIL) 
	{
		return FAIL;
	}

	/*�����㷨��ʼ*/
	do {
		// 1.�ļ��ָ��2048�ֽڴ�С�����ݿ飬��ȡһ�����ݿ�
		punit_inbuf_tmp = unit_inbuf_tmp;
		punit_inbuf = unit_inbuf;
		nBlockRead = 0;
		while (nBlockRead < sizeof(unit_inbuf)) 
		{ /*ѭ����ȡ2048�ֽ�����*/
			while (((nBytesRead=read(hFile_in,punit_inbuf_tmp,sizeof(unit_inbuf)-nBlockRead))==-1)&&(errno==EINTR))
				;
			if (nBytesRead < 0) 
			{
				close(hFile_in);
				return FAIL;
			} 
			else if (nBytesRead == 0) 
			{ /* read file end. */
				bFileEnd = TRUE;
				break;
			} 
			else if (nBytesRead < (sizeof(unit_inbuf)-nBlockRead)) 
			{
				memcpy(punit_inbuf, punit_inbuf_tmp, nBytesRead);
				punit_inbuf_tmp += nBytesRead;
				punit_inbuf += nBytesRead;
			} 
			else 
			{
				memcpy(punit_inbuf, punit_inbuf_tmp, nBytesRead);
			}
			nBlockRead += nBytesRead;
		}

		if (bFileEnd == TRUE) 
		{ 
			if ((nBlockRead != 0)&&(nBlockRead != sizeof(unit_inbuf))) 
			{
				close(hFile_in);
				return FAIL;
			} 
			else 
				if (nBlockRead == 0)/*the last block of file is null.*/
					break; 
		} 
		else 
		{
			if (nBlockRead != sizeof(unit_inbuf)) 
			{

				close(hFile_in);
				return FAIL;
			}
		}
		nFileRead += nBlockRead;

		// 2.����һ�����ݿ�(2048�ֽ�)
		punit_inbuf = unit_inbuf;
		punit_outbuf = unit_outbuf;
		for (i=0; i<(sizeof(unit_inbuf)/sizeof(des_inbuf)); i++) 
		{
			memcpy(des_inbuf, punit_inbuf, sizeof(des_inbuf)); /*��ʼ��des_inbuf[] */
			punit_inbuf += sizeof(des_inbuf); /*�ƶ�punit_inbufָ��*/

			if (i == 0) 
			{ /*��8���ֽڲ���TDES�㷨����*/
				iret = mdes3(DISCRYPT, des_inbuf, des_outbuf, key);
				if (iret == FAIL) 
				{
					close(hFile_in);
					return FAIL;
				}
			} 
			else 
			{ /*����ÿ8���ֽڲ�����ǰ8���ֽڵļ��ܽ������XOR */
				for (j=0; j<sizeof(des_inbuf); j++) 
				{
					des_outbuf[j] = xor_inbuf_pre[j] ^ des_inbuf[j];
				}
			}

			memcpy(punit_outbuf, des_outbuf, sizeof(des_outbuf));
			punit_outbuf += sizeof(des_outbuf); /*�ƶ�punit_outbufָ��*/
			memcpy(xor_inbuf_pre, des_inbuf, sizeof(des_inbuf));/*����xor_inbuf_pre[]*/
		}

		// 3.����ǵ�һ�����ݿ飬��ȡͷ����Ϣ�ṹ
		if (bFileStart == TRUE) 
		{
			memcpy(chartoint.c,unit_outbuf+HEADINFO_SIZE_OFF,sizeof(unsigned int));
			iHeaderLen=sizeof(NLD_HEADERINFO);
			iHeaderLen = chartoint.i;

			if (iHeaderLen != sizeof(NLD_HEADERINFO)) 
			{
				close(hFile_in);
				return FAIL;
			}
			memcpy(pHeaderInfo, unit_outbuf, iHeaderLen);
			break;
		}
		// 4.��������һ�����ݿ飬ȥ������ַ�
		if (nFileRead == iNldFileLen) 
		{
			nBlockRead = nBlockRead-(iNldFileLen-iHeaderLen-
				pHeaderInfo->uiTarFileLen-pHeaderInfo->uiSignatureLen);
		}

		bFileStart = FALSE; /* �Ժ�Ϊ��*/
	} while (1);
	
	close(hFile_in);
	return SUCC;
}

int SavePrikeyModulus_From_Array(R_RSA_PUBLIC_KEY *pPrivateKey)
{
	unsigned char *ptmp_mod=NULL, *p_mod=NULL;
	int i, j, temp, ch1,nArraySize; //D
	unsigned char szModulus[]="D605815CE2AD8275C18A39040D3F8E0CC3CA6620A6CA66CEDCA3C781F6D810B37FC4E5456993266FBF3054BEE1957B2E733C04721E2F87D3F504B94879DB1AAA7CEA9C5C12A36272E1584C1F30DE3DD76A206D492149A882CBAB4CDD44F2411FAC8EE2A9C2C3621C7A0EE3F983AAC5F5A649B61EFFB506F9EC0374E9187D4489DE528DC28D04874EC239E6720CD35954825CE8DB0F59656B7B7CF1C2E8F9CC640738CD8C4E43B450CA061FE4FA2AFADA7B0946CFAD3C160C283037FCC0B97FDE48D5DDE3D62FA483206015DF88A2981C7923007F7234AC96AA945891615E4C9BAA477B53C01A0A83493FA388FABADE6F42064A54BC48DFD920C721998FD3DEE1";
	
	nArraySize=sizeof(szModulus)-1;  //�����βΪ\0���ռ������512+1
	//save mod_bits
	pPrivateKey->bits=((nArraySize+1)/2)*8;//
	//CFunCommon::DlSprintf("pPrivateKey->bits=%d\n", pPrivateKey->bits);
	
	ptmp_mod=(unsigned char *)malloc(nArraySize);
	if(ptmp_mod==NULL)
	{
		CFunCommon::DlSprintf("Failed:malloc for private mod error!\n");
		return FAIL;
	}
	
	memcpy(ptmp_mod,szModulus,nArraySize);
	
	//���ַ�����ʽ��ģת��ΪBCD��
	p_mod=pPrivateKey->modulus+MAX_RSA_MODULUS_LEN-(nArraySize+1)/2;
	for(i=0,j=0; (i<nArraySize)&&(j<(nArraySize+1)/2); i++,j++)
	{
		if((i==0)&&(nArraySize%2 != 0))
		{//��Կ�ļ��к��������ַ�
			//��һ��ֻ��һ���ַ�
			if('A' <= ptmp_mod[i])/* A~F, a~f */
			{
				if('a' <= ptmp_mod[i])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_mod[i]-=32;
				}
				temp=ptmp_mod[i]-65+10;
				p_mod[j]=(0<<4)+((temp%16)&0x0F);
			}
			else
			{
				temp=ptmp_mod[i]-48;
				p_mod[j]=(0<<4)+((temp%10)&0x0F);
			}
		}
		else/*ÿ����2���ַ����һ��BCD��*/
		{
			if('A' <= ptmp_mod[i])/* A~F, a~f */
			{
				if('a' <= ptmp_mod[i])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_mod[i]-=32;
				}
				temp=ptmp_mod[i]-65+10;
				ch1=((temp%16)&0x0F)<<4;
			}
			else
			{
				temp=ptmp_mod[i]-48;
				ch1=((temp%10)&0x0F)<<4;
			}
			
			if('A' <= ptmp_mod[i+1])/* A~F, a~f */
			{
				if('a' <= ptmp_mod[i+1])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_mod[i+1]-=32;
				}
				temp=ptmp_mod[i+1]-65+10;
				p_mod[j]=ch1+((temp%16)&0x0F);
			}
			else
			{
				temp=ptmp_mod[i+1]-48;
				p_mod[j]=ch1+((temp%10)&0x0F);
			}
			
			i++;/*add 2*/
		}
	}

	free(ptmp_mod);
	
	return SUCC;
}

int SavePrivkeyExponent_From_Array(R_RSA_PUBLIC_KEY *pPrivateKey)
{
	unsigned char *ptmp_exp=NULL, *p_exp=NULL;
	int i, j, temp, ch1,nArraySize;
	unsigned char szExponent[]="C210A7E6B3DCA569659F9267AC23AF08C018AD567A2719E60587B50691F005E4DDBC8A30D0B48FABD06F60C8DD00907BE41DCDE234A0E0C73F6B931EB6F008540385315703C78723A8564A60160DCF819F47DC10EDD03EAEAB439F251CB99677C41EA4454EC7A01536507E43E83257E291705165F6740DE95D2B1D3DA7E91ED543EE1352410FB352E42A982980D1C468E29B78CDA510D750AE6EE5EF4F9148D6AF4ED532E68BEF6BEE44142F27EBA266C4E6AF4D35DEC70D82C049D68889263C02C0D3CADC454E7A418AF59ED10034ABACBAB306421A074B5BA44C085637C78C19CFB3A4C2DC215A774D818A84D8F779470828C18B6D1E5E4DF760630F1E0AA9";
	
	nArraySize=sizeof(szExponent)-1;
	
	ptmp_exp=(unsigned char *)malloc(nArraySize);
	if(ptmp_exp==NULL)
	{
		return FAIL;
	}
	
	//save exponent
	memcpy(ptmp_exp,szExponent,nArraySize);
	
	//���ַ�����ʽ��ģת��ΪBCD��
	p_exp=pPrivateKey->exponent+MAX_RSA_MODULUS_LEN-(nArraySize+1)/2;
	for(i=0,j=0; (i<nArraySize)&&(j<(nArraySize+1)/2); i++,j++)
	{
		if((i==0)&&(nArraySize%2 != 0))//��Կ�ļ��к��������ַ�
		{
			//��һ��ֻ��һ���ַ�
			if('A' <= ptmp_exp[i])/* A~F, a~f */
			{
				if('a' <= ptmp_exp[i])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_exp[i]-=32;
				}
				temp=ptmp_exp[i]-65+10;
				p_exp[j]=(0<<4)+((temp%16)&0x0F);
			}
			else
			{
				temp=ptmp_exp[i]-48;
				p_exp[j]=(0<<4)+((temp%10)&0x0F);
			}
		}
		else/*ÿ����2���ַ����һ��BCD��*/
		{
			if('A' <= ptmp_exp[i])/* A~F, a~f */
			{
				if('a' <= ptmp_exp[i])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_exp[i]-=32;
				}
				temp=ptmp_exp[i]-65+10;
				ch1=((temp%16)&0x0F)<<4;
			}
			else
			{
				temp=ptmp_exp[i]-48;
				ch1=((temp%10)&0x0F)<<4;
			}
			
			if('A' <= ptmp_exp[i+1])/* A~F, a~f */
			{
				if('a' <= ptmp_exp[i+1])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_exp[i+1]-=32;
				}

				temp=ptmp_exp[i+1]-65+10;
				p_exp[j]=ch1+((temp%16)&0x0F);
			}
			else
			{
				temp=ptmp_exp[i+1]-48;
				p_exp[j]=ch1+((temp%10)&0x0F);
			}
			i++;
		}
	}

	free(ptmp_exp);

	return SUCC;
}

/*
 *@brief	打包NLD
 *@param[in] pszDev 设备号： ttyACM/ttyUSB
 *@return
 *成功, 返回 true;
 *失败, 返回 false;
*/

int CreateParaNld(const char *pcMainNLD,const char *pcPackFile,  int nPara,int nIndex)
{
	char szCmd[256]={0};
	string strSourTargetPath,csTarFile,strUnnzpPath;
	NLD_HEADERINFO stHeaderinfo;
	struct stat stat_buf;

	
	strUnnzpPath=g_szAppPath;
	strUnnzpPath +=  "/unnzp/";

	char csNewNldFile[512] = {0};
	sprintf(csNewNldFile,"%s/unnzp/para_%d.%d.NLD",g_szAppPath,nIndex,nPara);
	memset(&stHeaderinfo,0x00,sizeof(NLD_HEADERINFO));
	
	strSourTargetPath = g_szAppPath;
	strSourTargetPath += "/unnzp/";
	strSourTargetPath += TAR_FILENAME;
	strSourTargetPath += ".gz";

	csTarFile=g_szAppPath;
	csTarFile += "/unnzp/";
	csTarFile += TAR_FILENAME;
	//pos端被打包文件当前只支持使用相对路径的tar，用绝对路径，pos端会报错。windows端不需要处理,mac命令行，所有参数已经转为绝对路径，此处转为相对路径
	string strTmp;
	int nPos;
	strTmp = pcPackFile;
	if((nPos = strTmp.rfind('/')) != string::npos)
		strTmp = strTmp.substr(nPos +1);
	CFunCommon::CopyFile((char *)pcPackFile,strTmp.c_str());
	chdir(strUnnzpPath.c_str());
	snprintf(szCmd,sizeof(szCmd),"tar cf %s %s",csTarFile.c_str(),strTmp.c_str());
	CFunCommon::ExecSystem(szCmd);
	chdir(g_szAppPath);
	
	//CreateTarFile(pcPackFile,csTarFile.c_str());
	snprintf(szCmd,sizeof(szCmd),"gzip -f %s",csTarFile.c_str());
	CFunCommon::ExecSystem(szCmd);
	//GzipFile(csTarFile.GetBuffer(0),strSourTargetPath.GetBuffer(0));
	
	GetHeadInfo(pcMainNLD,&stHeaderinfo);


	stHeaderinfo.cType='P';

	if(stat(strSourTargetPath.c_str(), &stat_buf) == FAIL)
	{

		return -1;
	}
	if (stat_buf.st_size == 0) 
	{

		return -1;
	}

	stHeaderinfo.uiTarFileLen = stat_buf.st_size;	//tar包的长度	
	stHeaderinfo.uiSignatureLen = 256;
	chdir(strUnnzpPath.c_str());
	CreateParaFile(&stHeaderinfo,csNewNldFile);
	chdir(g_szAppPath);

	return 0;
}


/****************************************************************
** �������ܣ�����NLD�ļ���
             ���ļ���ͷ����Ϣ��Tar�ļ���ɡ�
**
** ����������
                    pHeaderInfo            ͷ����Ϣ�ṹ��ָ��
** ����ֵ  ��
                    SUCC        �ɹ�
                    FAIL        ʧ��
** ȫ�ֱ�����
** ����ģ�飺��
** ˵    ����1. ���ļ�����������ϢժҪ��
             2. ���ļ���������ϢժҪ�����������ܺ�
                �γɿ��Է�������ʽ��NLD����
****************************************************************/
int CreateParaFile(const NLD_HEADERINFO *pHeaderInfo,const char *pcOutName)
{
	int hFileNld, hFileTar;
	char buf[1024], *pbuf=NULL;
	int nBytesRead, nBytesWrite;

	unsigned char pOutput[2048]={0};
	unsigned int OutputLen = 0;
	R_RSA_PUBLIC_KEY privateKey;
	unsigned char strDigest[20];
	int isKey=0;
	int iRet=0;

	// 1.create the file "a.nld", include the header info and the tar file.
	
	if((hFileNld=open(NLD_FILENAME,O_RDWR|O_CREAT|O_TRUNC,0666))==FAIL) /*���������ļ�*/
	{
		return FAIL;
	}

	// 2.write header info to a.nld.
	pbuf = (char *)pHeaderInfo;
	nBytesRead = sizeof(NLD_HEADERINFO);
	while(nBytesRead > 0)
	{
		while(((nBytesWrite=write(hFileNld,pbuf,nBytesRead))==FAIL)&&(errno==EINTR))
			;
		if (nBytesWrite < 0)
		{
			close(hFileNld);
			remove(NLD_FILENAME);
			return FAIL;
		}
		if (nBytesWrite == 0)
			break;
		nBytesRead -= nBytesWrite;
		pbuf += nBytesWrite;
	}
	if (nBytesRead != 0)
	{
		close(hFileNld);
		remove(NLD_FILENAME);
		return FAIL;
	}

	// 3.write dl.tar.gz to a.nld
	if( (hFileTar = open(TAR_GZ_FILENAME, O_RDONLY)) == FAIL)
	{
		close(hFileNld);
		remove(NLD_FILENAME);
		return FAIL;
	}

	while(1)
	{
		while(((nBytesRead=read(hFileTar,buf,sizeof(buf)))==-1)&&(errno==EINTR))
			;
		if (nBytesRead < 0)
		{
			close(hFileTar);
			close(hFileNld);
			remove(NLD_FILENAME);
			return FAIL;
		}
		if (nBytesRead == 0)
			break;

		//write data to hFileNld.
		pbuf = buf;
		while(nBytesRead > 0)
		{
			while(((nBytesWrite=write(hFileNld,pbuf,nBytesRead))==-1)&&(errno==EINTR))
				;
			if (nBytesWrite < 0)
			{
				CFunCommon::DlSprintf("Failed:ERR: Write %s failed. nBytesWrite=%d\n", TAR_FILENAME, nBytesWrite);
				close(hFileTar);
				close(hFileNld);
				remove(NLD_FILENAME);
				return FAIL;
			}
			if (nBytesWrite == 0)
				break;
			nBytesRead -= nBytesWrite;
			pbuf += nBytesWrite;
		}
		if (nBytesRead != 0)
		{
			close(hFileTar);
			close(hFileNld);
			remove(NLD_FILENAME);
			return FAIL;
		}
	}
	close(hFileTar);
	close(hFileNld);
	CIniRW ini;
	ini.iniFileLoad("../para.ini");
	isKey=ini.iniGetInt("PARA","ISKEY",0);

	//isKey=GetPrivateProfileInt("PARA","ISKEY",0,"..\\para.ini");

	if (!isKey)//ֻ��Ϊ�˼������ڵĻ�������ֹ����������Ҫ��ǩ��������Ԥ��
	{
		OutputLen=256;
		memset(pOutput,0xaa,sizeof(pOutput));
	}
	else
	{
		//SavePrikeyModulus("../pri_mod",&privateKey);
		//SavePrivkeyExponent("../pri_exp",&privateKey);
		SavePrikeyModulus_From_Array(&privateKey);
		SavePrivkeyExponent_From_Array(&privateKey);

		ComputeDigest(NLD_FILENAME,strDigest);
		
		CCodeTools::RSAPublicEncrypt(pOutput, &OutputLen, strDigest, 20, &privateKey);
		if(OutputLen != (privateKey.bits+7)/8)
		{
			return -1;
		}
	}
	
	// 8.��a.nld�ļ�β������ǩ��������δ���ܵ�NLD��
	//open a.tmpnld
	if((hFileNld=open(NLD_FILENAME,O_RDWR|O_APPEND,0666))==FAIL)
	{
		return -1;
	}
	
	//write signature to a.tmpnld
	nBytesWrite=0;
	nBytesRead = OutputLen;
	while(nBytesRead > 0)
	{
		while(((iRet=write(hFileNld,pOutput+nBytesWrite,nBytesRead))==FAIL)&&(errno==EINTR))
			;
		if (iRet < 0)
		{
			close(hFileNld);
			return -1;
		}

		if (iRet == 0)
			break;

		nBytesRead -= iRet;
		nBytesWrite+=iRet;
	}

	if (nBytesRead != 0)
	{
		close(hFileNld);
		return -1;
	}

	close(hFileNld);
	
	// 9.���ܣ�������ʽ��NLD��
	
	// 3DES
	iRet = FileEncrypt(NLD_FILENAME, pcOutName);
	if(iRet == FAIL)
	{
		CFunCommon::DlSprintf("Failed:File Decrypt error.\n");
		return -1;
	}

	return SUCC;
}


//����������ļ�������ǩ��
int ComputeDigest ( const char *pFilename, unsigned char *pDigest )
{
    FILE *fp = NULL;
    unsigned char buffer[4096],sha1val[20];
	sha1_context ctx;
	int len;
	
    fp = fopen (pFilename, "rb");
    if (!fp)
    {
        return -1;
    }
	
	CCodeTools::sha1_starts(&ctx);
	while ((len = fread (buffer, 1, 4096, fp))) 
	{
		CCodeTools::sha1_update(&ctx, buffer, len);
	}
	CCodeTools::sha1_finish(&ctx, sha1val);
	
    fclose (fp);
	fp=NULL;
	
    memcpy(pDigest, sha1val, 20);
	
    return 0;
}


void DeleFile(const char *szFileName)
{
	remove(szFileName);
}


/****************************************************************
** �������ܣ��ļ�����
**
** ���������
                    pInFilename     �û������Ҫ���ܵ��ļ���
** ���������
                    pOutFilename    �õ����ܺ���Ҫ���ļ���
** ����ֵ  ��
                    SUCC        �ɹ�
                    FAIL        ʧ��
** ȫ�ֱ�����
** ����ģ�飺
** ˵    ����
        ����ʵ���㷨�μ�:
       ��NL-GP730-5-The secure mechanism of software download V2.2.doc��
****************************************************************/
int FileEncrypt(const char *pInFilename, const char *pOutFilename)
{
	int hFileIn, hFileOut;
	int nBytesRead, nBlockRead, nBytesWrite;
	unsigned char unit_inbuf_tmp[UNIT_BUF_SIZE], unit_inbuf[UNIT_BUF_SIZE];
	unsigned char unit_outbuf[UNIT_BUF_SIZE];
	unsigned char des_inbuf[TDES_INPUT_SIZE], des_outbuf[TDES_INPUT_SIZE];
	unsigned char xor_inbuf_pre[TDES_INPUT_SIZE];
	unsigned char *punit_inbuf_tmp=NULL, *punit_inbuf=NULL, *punit_outbuf=NULL;
	unsigned char key[16] = { 1, 3, 5, 7, 2, 4, 6, 8, 1, 2, 3, 4, 5, 6, 7, 8 };
	int i, j;
	int bFileEnd = FALSE;

	if( (hFileIn = open(pInFilename, O_RDONLY)) == FAIL)
	{
		return FAIL;
	}

	if( (hFileOut=open(pOutFilename, O_RDWR|O_CREAT|O_TRUNC, 0666))==FAIL)
	{
		close(hFileIn);
		return FAIL;
	}

	/*�����㷨��ʼ*/
	while(1)
	{
		// 1.�ļ��ָ��2048�ֽڴ�С�����ݿ飬��ȡһ�����ݿ�
		punit_inbuf_tmp = unit_inbuf_tmp;
		punit_inbuf = unit_inbuf;
		nBlockRead = 0;
		while (nBlockRead < sizeof(unit_inbuf)) /*ѭ����ȡ2048�ֽ�����*/
		{
			while(((nBytesRead=read(hFileIn,punit_inbuf_tmp,sizeof(unit_inbuf)-nBlockRead))==-1)&&(errno==EINTR))
				;
			if (nBytesRead < 0) 
			{
				close(hFileIn);
				close(hFileOut);
				DeleFile(pOutFilename);
				return FAIL;
			} 
			else if (nBytesRead == 0) 
			{ /* read file end. */
				bFileEnd = TRUE;
				break;
			} 
			else if (nBytesRead < (sizeof(unit_inbuf)-nBlockRead)) 
			{
				memcpy(punit_inbuf, punit_inbuf_tmp, nBytesRead);
				punit_inbuf_tmp += nBytesRead;
				punit_inbuf += nBytesRead;
			} 
			else 
			{
				memcpy(punit_inbuf, punit_inbuf_tmp, nBytesRead);
			}
			nBlockRead += nBytesRead;
		}

		if (bFileEnd == TRUE) /*the last block of file. */
		{
			if (nBlockRead != 0) 
			{ /*the last block of file have some data. */
				punit_inbuf = unit_inbuf;
				memset(punit_inbuf+nBlockRead,0,sizeof(unit_inbuf)-nBlockRead);/*fill 0x00.*/
			}
			else 
			{ /*the last block of file is null. */
				break; /* sucess. */
			}
		}
		else
		{
			if (nBlockRead != sizeof(unit_inbuf)) 
			{
				close(hFileIn);
				close(hFileOut);
				DeleFile(pOutFilename);
				return FAIL;
			}
		}

		// 2.����һ�����ݿ�(2048�ֽ�)
		punit_inbuf = unit_inbuf;
		punit_outbuf = unit_outbuf;
		for (i=0; i<(sizeof(unit_inbuf)/sizeof(des_inbuf)); i++) /* handle 8 characters each time. */
		{
			memcpy(des_inbuf, punit_inbuf, sizeof(des_inbuf)); /*��ʼ��des_inbuf[] */
			punit_inbuf += sizeof(des_inbuf); /*�ƶ�punit_inbufָ��*/

			if (i == 0) /*��8���ֽڲ���TDES�㷨����*/
			{
				mdes3(0 , des_inbuf, des_outbuf, key);
			}
			else /*����ÿ8���ֽڲ�����ǰ8���ֽڵļ��ܽ������XOR */
			{
				for (j=0; j<sizeof(des_inbuf); j++)
				{
					des_outbuf[j] = xor_inbuf_pre[j] ^ des_inbuf[j];
				}
			}

			memcpy(punit_outbuf, des_outbuf, sizeof(des_outbuf));
			punit_outbuf += sizeof(des_outbuf); /*�ƶ�punit_outbufָ��*/
			memcpy(xor_inbuf_pre, des_outbuf, sizeof(des_inbuf)); /*����xor_inbuf_pre[] */
		}

		// 3.��һ�����ݿ�д��hFile2�ļ�
		punit_outbuf = unit_outbuf;
		nBytesRead = sizeof(unit_inbuf);
		while(nBytesRead > 0)
		{
			while(((nBytesWrite=write(hFileOut,punit_outbuf,nBytesRead))==-1)&&(errno==EINTR))
				;
			if (nBytesWrite < 0)
			{
				close(hFileIn);
				close(hFileOut);
				DeleFile(pOutFilename);
				return FAIL;
			}

			if (nBytesWrite == 0)
				break;
			nBytesRead -= nBytesWrite;
			punit_outbuf += nBytesWrite;
		}

		if (nBytesRead != 0)
		{
			close(hFileIn);
			close(hFileOut);
			DeleFile(pOutFilename);
			return FAIL;
		}
	}

	close(hFileIn);
	close(hFileOut);

	return SUCC;
}


#if 0


/****************************************************************
** �������ܣ��ļ�����
**
** ���������
pInFilename  	    �û������Ҫ���ܵ��ļ���

  ** ���������
  pOutFilename    	�õ����ܺ���Ҫ���ļ���
  pOutFileLen     	���ܺ��NLD���ļ�����
  pHeaderInfo   	NLD����ͷ����Ϣ�ṹָ��
  ** ����ֵ  ��
  SUCC      		�ɹ�
  FAIL        		ʧ��
  ** ȫ�ֱ�����
  ** ����ģ�飺
  ** ˵    ������
****************************************************************/

int FileDecrypt(const char *pInFilename, const char *pOutFilename,  NLD_HEADERINFO *pHeaderInfo)
{
	int hFile_in, hFile_out;
	int nBytesRead, nBytesWrite, nBlockRead, nFileRead = 0;
	int iret;
	unsigned char unit_inbuf_tmp[UNIT_BUF_SIZE], unit_inbuf[UNIT_BUF_SIZE];
	unsigned char unit_outbuf[UNIT_BUF_SIZE];
	unsigned char des_inbuf[TDES_INPUT_SIZE], des_outbuf[TDES_INPUT_SIZE];
	unsigned char xor_inbuf_pre[TDES_INPUT_SIZE];
	unsigned char *punit_inbuf_tmp = NULL, *punit_inbuf = NULL;
	unsigned char *punit_outbuf = NULL;
	unsigned char key[16] = {1, 3, 5, 7, 2, 4, 6, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	int i, j;
	int bFileStart = TRUE, bFileEnd = FALSE;
	unsigned int iHeaderLen = 0, iNldFileLen;
	struct stat stat_buf;
	DATA1 chartoint;

	if (stat(pInFilename, &stat_buf) == FAIL) 
	{
		return FAIL;
	}
	iNldFileLen = stat_buf.st_size;

	if ((hFile_in = open(pInFilename, O_RDONLY)) == FAIL) 
	{

		return FAIL;
	}

	if ((hFile_out = open(pOutFilename, O_RDWR|O_CREAT|O_TRUNC, 0666)) == FAIL) 
	{

		close(hFile_in);
		return FAIL;
	}

	//�����㷨��ʼ
	do {
		// 1.�ļ��ָ��2048�ֽڴ�С�����ݿ飬��ȡһ�����ݿ�
		punit_inbuf_tmp = unit_inbuf_tmp;
		punit_inbuf = unit_inbuf;
		nBlockRead = 0;
		while (nBlockRead < sizeof(unit_inbuf)) 
		{ //ѭ����ȡ2048�ֽ�����
			while (((nBytesRead=read(hFile_in,punit_inbuf_tmp,sizeof(unit_inbuf)-nBlockRead))==-1)&&(errno==EINTR))
				;
			if (nBytesRead < 0) 
			{
				close(hFile_in);
				close(hFile_out);
				remove(pOutFilename);
				return FAIL;
			} 
			else if (nBytesRead == 0) 
			{ // read file end.
				bFileEnd = TRUE;
				break;
			} 
			else if (nBytesRead < (sizeof(unit_inbuf)-nBlockRead)) 
			{
				memcpy(punit_inbuf, punit_inbuf_tmp, nBytesRead);
				punit_inbuf_tmp += nBytesRead;
				punit_inbuf += nBytesRead;
			} 
			else 
			{
				memcpy(punit_inbuf, punit_inbuf_tmp, nBytesRead);
			}
			nBlockRead += nBytesRead;
		}

		if (bFileEnd == TRUE) 
		{ //the last block of file.
			if ((nBlockRead != 0)&&(nBlockRead != sizeof(unit_inbuf))) 
			{
				//the last block of file have some data

				close(hFile_in);
				close(hFile_out);
				remove(pOutFilename);
				return FAIL;
			} 
			else 
				if (nBlockRead == 0)//the last block of file is null.
					break; // sucess
		} 
		else 
		{
			if (nBlockRead != sizeof(unit_inbuf)) 
			{

				close(hFile_in);
				close(hFile_out);
				remove(pOutFilename);
				return FAIL;
			}
		}
		nFileRead += nBlockRead;

		// 2.����һ�����ݿ�(2048�ֽ�)
		punit_inbuf = unit_inbuf;
		punit_outbuf = unit_outbuf;
		for (i=0; i<(sizeof(unit_inbuf)/sizeof(des_inbuf)); i++) {
			memcpy(des_inbuf, punit_inbuf, sizeof(des_inbuf)); //��ʼ��des_inbuf[]
			punit_inbuf += sizeof(des_inbuf); //�ƶ�punit_inbufָ��

			if (i == 0) { //��8���ֽڲ���TDES�㷨����
				iret = mdes3(DISCRYPT, des_inbuf, des_outbuf, key);
				if (iret == FAIL) 
				{
				//	WriteLog(download_log, "ERR: Call mdes3 failed.");
					close(hFile_in);
					close(hFile_out);
					remove(pOutFilename);
					return FAIL;
				}
			} 
			else 
			{ //����ÿ8���ֽڲ�����ǰ8���ֽڵļ��ܽ������XOR
				for (j=0; j<sizeof(des_inbuf); j++) 
				{
					des_outbuf[j] = xor_inbuf_pre[j] ^ des_inbuf[j];
				}
			}

			memcpy(punit_outbuf, des_outbuf, sizeof(des_outbuf));
			punit_outbuf += sizeof(des_outbuf); //�ƶ�punit_outbufָ��
			memcpy(xor_inbuf_pre, des_inbuf, sizeof(des_inbuf));//����xor_inbuf_pre[]
		}

		// 3.����ǵ�һ�����ݿ飬��ȡͷ����Ϣ�ṹ
		if (bFileStart == TRUE) 
		{
			// get the size of headerInfo.
			memcpy(chartoint.c,unit_outbuf+HEADINFO_SIZE_OFF,sizeof(unsigned int));
			iHeaderLen = chartoint.i;

			if (iHeaderLen != sizeof(NLD_HEADERINFO)) 
			{
				//WriteLog(download_log, "iHeaderLen!=sizeof(NLD_HEADERINFO)");
				close(hFile_in);
				close(hFile_out);
				remove(pOutFilename);
				return FAIL;
			}
			memcpy(pHeaderInfo, unit_outbuf, iHeaderLen);
		}

		// 4.��������һ�����ݿ飬ȥ������ַ�
		if (nFileRead == iNldFileLen) 
		{
			nBlockRead = nBlockRead-(iNldFileLen-iHeaderLen-
				pHeaderInfo->uiTarFileLen-pHeaderInfo->uiSignatureLen);
		}

		// 5.�����ݿ�д��hFile2�ļ�
		punit_outbuf = unit_outbuf;
		while (nBlockRead > 0) 
		{
			while (((nBytesWrite=write(hFile_out,punit_outbuf,nBlockRead))==-1)&&(errno==EINTR))
				;
			if (nBytesWrite < 0) 
			{
				close(hFile_in);
				close(hFile_out);
				remove(pOutFilename);
				return FAIL;
			}
			if (nBytesWrite == 0)
				break;
			nBlockRead -= nBytesWrite;
			punit_outbuf += nBytesWrite;
		}

		if (nBlockRead != 0) 
		{
			close(hFile_in);
			close(hFile_out);
			remove(pOutFilename);
			return FAIL;
		}

		bFileStart = FALSE; ///* �Ժ�Ϊ��
	} while (1);

	if (nFileRead != iNldFileLen) 
	{

	}

	close(hFile_in);
	close(hFile_out);

	remove(pOutFilename);

	return SUCC;
}  


/****************************************************************
** �������ܣ�����NLD�ļ�
**
** ���������
char *pcPackageFile  	    Ҫ������ļ����б�,�ļ�֮����һ���ո�����
char *pHeaderName			ͷ�ļ�

  ** ���������
  pcOutNldName    	������ɵ�nld�ļ���
  ** ����ֵ  ��
  SUCC      		�ɹ�
  FAIL        		ʧ��
  ** ȫ�ֱ�����
  ** ����ģ�飺
  ** ˵    ������
****************************************************************/
int MakeNLD(char *pcPackageFile,char *pHeaderName,char *pcOutNldName)
{

	return SUCC;
}




/****************************************************************
** �������ܣ���ȡini �ļ��е����Keyֵ
**
** ����������
                    pFilename        ini �ļ���
                    pSection         Section�ؼ����ַ���
                    pKey             Key�ؼ����ַ���
** ����ֵ  ��
                    strTmp           ��ȡ��Keyֵ�ַ�����ָ��
                    NULL             ��ȡʧ��
** ȫ�ֱ�����
** ����ģ�飺��
** ˵    ������������NULL���������:
             1. �ļ���ʧ�ܣ�
             2. Key��Ӧ��ֵΪ�գ������Ҳ�����
****************************************************************/
char* GetInitKey(const char *pFilename, char *pSection, char *pKey)
{
	FILE *fp=NULL;
	char TmpLine[1024]; /*��Ŵ��ļ���ȡ��һ������*/
	int rtnval, cmp_ret;
	int i = 0;
	int flag = 0;
	char *pTmp=NULL;
	static char strTmp[1024]; /*�����ʱ�ִ��������ص��ִ���������static����*/

	if ((fp = fopen(pFilename, "rb")) == NULL) /*ע���ļ��Ĵ򿪷�ʽ*/
	{
		CFunCommon::DlSprintf("Cannot open %s\n", pFilename);
		return NULL;
	}

	while (!feof(fp)) /*�Ƿ񵽴��ļ�β*/
	{
		rtnval = fgetc(fp); /*���ļ�����ȡһ���ַ�*/
		if (rtnval == EOF) 
		{		
			break;
		} else {
			TmpLine[i++] = rtnval;
		}
		if (rtnval == '\n') { /*�س����з���ASCII��Ϊ0x0D 0x0A����'\r' '\n'������'\n'== 0x0A */
			//TmpLine[--i] = 0; /*����TmpLine[] ����Ч�ַ��������2���ַ�Ϊ0x0D 0x00 */
			i = i - 2;
			TmpLine[i] = 0; /*����TmpLine[] ����Ч�ַ��������2���ַ�Ϊ0x00 0x0A */
#ifdef DEBUG_GET_INI
			DEBUG_DUMP("\nTmpLine= %s\n", TmpLine);
			DEBUG_DUMP("i=%d, strlen(TmpLine)=%d\n", i, strlen(TmpLine));
			DEBUG_DUMP("TmpLine= %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
				TmpLine[0], TmpLine[1], TmpLine[2], TmpLine[3], TmpLine[4], TmpLine[5], TmpLine[6],
				TmpLine[7], TmpLine[8], TmpLine[9], TmpLine[10], TmpLine[11], TmpLine[12], TmpLine[13],
				TmpLine[14], TmpLine[15] );
#endif
			i = 0; /* TmpLine[] �´α����µ�һ��*/
			pTmp = strchr(TmpLine, '='); /*�����ִ�tmpLine�е�1�γ����ַ�'='��λ��ָ��*/
			if ((pTmp != NULL) && (flag == 1)) {
#ifdef DEBUG_GET_INI
				DEBUG_DUMP("*pTmp= '%c',pKey=\"%s\",strlen(pKey)=%d\n", *pTmp, pKey, strlen(pKey));
#endif
				if (strstr(TmpLine, pKey) != NULL) { /*����pkey��TmpLine�е�1�γ��ֵ�λ��ָ��*/
						strcpy(strTmp, pTmp+1);
#ifdef DEBUG_GET_INI
						DEBUG_DUMP("strTmp=%s, strlen(strTmp)=%d\n", strTmp, strlen(strTmp)); /*�ִ�strTmp�а���'\0'*/
#endif
						fclose(fp);
						return strTmp; /*warning: return address of local variable. */
				}
			} else {
				strcpy(strTmp, "["); /*���ִ�"["���Ƶ��ִ�strTmp */
				strcat(strTmp, pSection); /*�����ִ�psection���ִ�strTmp֮��*/
				strcat(strTmp, "]");
#ifdef DEBUG_GET_INI
				DEBUG_DUMP("strTmp=  %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
					strTmp[0], strTmp[1], strTmp[2], strTmp[3], strTmp[4], strTmp[5], strTmp[6],
					strTmp[7], strTmp[8], strTmp[9], strTmp[10], strTmp[11], strTmp[12], strTmp[13],
					strTmp[14], strTmp[15] );
#endif
				cmp_ret = strcmp(strTmp, TmpLine);
#ifdef DEBUG_GET_INI
				DEBUG_DUMP("strTmp=%s,strlen(strTmp)=%d,cmp_ret=%d\n",
					strTmp, strlen(strTmp), cmp_ret);
#endif
				if (cmp_ret == 0) { /*�Ƚ϶α�����strTmp�����ִ�tmpLine*/
					flag = 1;
				}
			}
		}
	}

	CFunCommon::DlSprintf("GetInitKey return, can't find the value of the '%s'.\n", pKey);
	fclose(fp);
	return NULL;
}


/****************************************************************
** �������ܣ����NLD�ļ���ͷ����Ϣ�ṹ
**
** ����������
                    pFilename             ini �ļ���
                    pHeaderInfo           ͷ����Ϣ�ṹ��ָ��
** ����ֵ  ��
                    SUCC                  �ɹ�
                    FAIL                  ʧ��
** ȫ�ֱ�����
** ����ģ�飺��
** ˵    ���������û��޷�֪��tar�ļ��Ĵ�С��������û�ж�
             HeaderInfo.uiTarFileLen���и�ֵ���ڴ���Tar�ļ�֮
             �����������غ�����ȡTar�ļ���С��Ȼ����и�ֵ��
             ���⣬δ��ʼ��ǩ���Ĵ�С��
****************************************************************/
int FillHeaderInfo(const char *pFilename, NLD_HEADERINFO *pHeaderInfo)
{
	char *pStrInfo = NULL;
	struct stat stat_buf;
	int i, j, k;
	char cTmpReverse[sizeof(unsigned long)+1]; /*�����ڱ���UserPrgInfo->sReverse[3]�е�һ��Ԫ��*/

	pHeaderInfo->uiHeadSize = sizeof(NLD_HEADERINFO); /*ͷ����Ϣ�ĳ����ɽṹ��HEADERINFO ����*/

	pStrInfo = GetInitKey(pFilename, "DownloadInfo", "strMachineType");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}

	strcpy(pHeaderInfo->cMachineType, pStrInfo); /*����������'\0' */

	pStrInfo = GetInitKey(pFilename, "DownloadInfo", "cType");
	
	if(pStrInfo == NULL)
	{
		pHeaderInfo->cType= 'P';
	}
	else
	{
		pHeaderInfo->cType= *pStrInfo;
	}

	
	pStrInfo = GetInitKey(pFilename, "DownloadInfo", "uiKeyExpiredDate");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}
	if(pStrInfo[0] == '\0') 
	{

	}
	pHeaderInfo->uiKeyExpiredDate = (unsigned int )strtoul(pStrInfo, NULL, 10);

	//Ϊ�ṹ��HeaderInfo�е�uiTarFileLen��Ա��ֵ
	if(stat(TAR_GZ_FILENAME, &stat_buf) == FAIL)
	{
		return FAIL;
	}

	if (stat_buf.st_size == 0) 
	{
		return FAIL;
	}
	pHeaderInfo->uiTarFileLen = stat_buf.st_size;	//tar���ĳ���

	pHeaderInfo->uiSignatureLen=256;

	//Section = [AppInfo]
	pStrInfo = GetInitKey(pFilename, "AppInfo", "strAppName");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}

	if(pStrInfo[0] == '\0') 
	{

	}
	strcpy(pHeaderInfo->UserPrgInfo.cAppName, pStrInfo);

	pStrInfo = GetInitKey(pFilename, "AppInfo", "strVerBuf");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}

	if(pStrInfo[0] == '\0') 
	{

	}
	strcpy(pHeaderInfo->UserPrgInfo.cVerBuf, pStrInfo);

	pStrInfo = GetInitKey(pFilename, "AppInfo", "iSeriNo");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}
	if(pStrInfo[0] == '\0') 
	{

	}
	pHeaderInfo->UserPrgInfo.cSeriNo = atoi(pStrInfo);

	pStrInfo = GetInitKey(pFilename, "AppInfo", "strBuildTime");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}

	if(pStrInfo[0] == '\0') 
	{

	}
	strcpy(pHeaderInfo->UserPrgInfo.tBuildTime, pStrInfo);

	pStrInfo = GetInitKey(pFilename, "AppInfo", "bIsMaster");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}

	if(pStrInfo[0] == '\0') 
	{

	}
	pHeaderInfo->UserPrgInfo.bIsMaster= atoi(pStrInfo);
	pStrInfo = GetInitKey(pFilename, "AppInfo", "sReverse");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}
	if(pStrInfo[0] == '\0') 
	{
		for (i=0,j=0; i<(sizeof(pHeaderInfo->UserPrgInfo.sReverse)/sizeof(unsigned long)); i++)
		{
			pHeaderInfo->UserPrgInfo.sReverse[i] = 0;
		}
	} 
	else 
	{
		for (i=0, j=0; i<(sizeof(pHeaderInfo->UserPrgInfo.sReverse)/sizeof(unsigned long)); i++)
		{
			k = 0; /*j ָ�����������������i ָ������Ԫ�ص�������k ָ������Ԫ���ڵ�����*/
			for( ; (pStrInfo[j] != ',')&&(pStrInfo[j] != '\0'); j++) /*����Ԫ��֮���ö���','����������֮����'\0' */
			{ 
				cTmpReverse[k] = pStrInfo[j];
				k++;
			}
			cTmpReverse[k] = '\0';

			if(cTmpReverse[0] != '\0') 
			{
				pHeaderInfo->UserPrgInfo.sReverse[i] = strtoul(cTmpReverse, NULL, 10);
			}

			if(pStrInfo[j] == '\0') 
			{ /*���*/
				break;
			} 
			else if(pStrInfo[j] == ',')
			{ /*����j ֵ������',' */
				j++;
			} 
			else 
			{
				return FAIL;
			}
		}
	}

	pStrInfo = GetInitKey(pFilename, "Extended", "strExtended");
	if(pStrInfo == NULL) 
	{
		return FAIL;
	}

	if(pStrInfo[0] == '\0') 
	{

	}
	strcpy(pHeaderInfo->cExtended, pStrInfo);

	return SUCC;
}


/****************************************************************
** �������ܣ��ӹ�Կģ�ļ��л�ȡ��Կ��ģ�����浽Ŀ���ļ���
**
** ����������
pFilename        ��Կ�ļ���
obj_fd           Ŀ���ļ�
** ����ֵ  ��
SUCC             �ɹ�
FAIL             ʧ��
** ȫ�ֱ�������
** ����ģ�飺��
** ˵    ������Կģ�ļ��е�������Ϣ����Ϊ��Կ��ģ��
****************************************************************/
int SavePubkeyModulus(const char *pFilename, int obj_fd)
{
	struct stat stat_buf;
	int hFileIn;
	unsigned char *pstr_mod;
	
	if(stat(pFilename, &stat_buf) == FAIL) {
		CFunCommon::DlSprintf("ERR: Get the size of file '%s' failed.\n", pFilename);
		return FAIL;
	}
	CFunCommon::DlSprintf("The size of file '%s' is %d\n", pFilename, (int)stat_buf.st_size);
	CFunCommon::DlSprintf("The mod_str_len is %d\n", (int)stat_buf.st_size);
	
	//save mod_str_len
	if(write(obj_fd,&(stat_buf.st_size),sizeof(int))!=sizeof(int))
	{
		CFunCommon::DlSprintf("write mod_str_len error!\n");
		return FAIL;
	}

	if((hFileIn = open(pFilename, O_RDONLY)) == FAIL)
	{
		CFunCommon::DlSprintf("ERR: cannot open file '%s'.\n", pFilename);
		return FAIL;
	}

	pstr_mod=(unsigned char *)malloc(stat_buf.st_size);
	if(pstr_mod==NULL){
		CFunCommon::DlSprintf("malloc for mod_str error!\n");
		close(hFileIn);
		return FAIL;
	}

	if(read(hFileIn,pstr_mod,stat_buf.st_size)!=stat_buf.st_size)
	{
		CFunCommon::DlSprintf("read mod_str error!\n");
		close(hFileIn);
		return FAIL;
	}
	close(hFileIn);

	//save mod_str
	if(write(obj_fd,pstr_mod,stat_buf.st_size)!=stat_buf.st_size)
	{
		CFunCommon::DlSprintf("write mod_str error!\n");
		return FAIL;
	}

#ifdef DEBUG_PACK
	int i;
	CFunCommon::DlSprintf("mod_str=");
	for(i=0;i<stat_buf.st_size; i++)
		CFunCommon::DlSprintf("%c.", pstr_mod[i]);
	CFunCommon::DlSprintf("\n");
#endif
	free(pstr_mod);

	return SUCC;
}


/****************************************************************
** �������ܣ��ӹ�Կָ���ļ��л�ȡ��Կ��ָ�������浽Ŀ���ļ���
**
** ����������
                    pFilename        ��Կ�ļ���
                    obj_fd           Ŀ���ļ�
** ����ֵ  ��
                    SUCC             �ɹ�
                    FAIL             ʧ��
** ȫ�ֱ�������
** ����ģ�飺��
** ˵    ������Կָ���ļ��е�������Ϣ����Ϊ��Կ��ָ����
****************************************************************/
int SavePubkeyExponent(const char *pFilename, int obj_fd)
{
	struct stat stat_buf;
	int hFileIn;
	unsigned char *pstr_exp;

	if(stat(pFilename, &stat_buf) == FAIL) {
		CFunCommon::DlSprintf("ERR: Get the size of file '%s' failed.\n", pFilename);
		return FAIL;
	}
	CFunCommon::DlSprintf("The size of file '%s' is %d\n", pFilename, (int)stat_buf.st_size);
	CFunCommon::DlSprintf("The exp_str_len is %d\n", (int)stat_buf.st_size);

	//save exp_str_len
	if(write(obj_fd,&(stat_buf.st_size),sizeof(int))!=sizeof(int)){
		CFunCommon::DlSprintf("write exp_str_len error!\n");
		return FAIL;
	}

	if((hFileIn = open(pFilename, O_RDONLY)) == FAIL)
	{
		CFunCommon::DlSprintf("ERR: cannot open file '%s'.\n", pFilename);
		return FAIL;
	}

	pstr_exp=(unsigned char *)malloc(stat_buf.st_size);
	if(pstr_exp==NULL)
	{
		CFunCommon::DlSprintf("malloc for exp_str error!\n");
		close(hFileIn);
		return FAIL;
	}

	if(read(hFileIn,pstr_exp,stat_buf.st_size)!=stat_buf.st_size){
		CFunCommon::DlSprintf("read exp_str error!\n");
		close(hFileIn);
		return FAIL;
	}
	close(hFileIn);

	//save exponent_str
	if(write(obj_fd,pstr_exp,stat_buf.st_size)!=stat_buf.st_size){
		CFunCommon::DlSprintf("write exp_str error!\n");
		return FAIL;
	}

#ifdef DEBUG_PACK
	int i;
	CFunCommon::DlSprintf("exp_str=");
	for(i=0;i<stat_buf.st_size; i++)
		CFunCommon::DlSprintf("%c.", pstr_exp[i]);
	CFunCommon::DlSprintf("\n");
#endif
	free(pstr_exp);

	return SUCC;
}


/****************************************************************
** �������ܣ�������Կ�ļ�������ʼ����Կ
**
** ���������
                  pKeyFilename      ��Կ�ļ���ָ��
                  pPublicKey        ��Կָ��
** ���������
                  ��
** ����ֵ  ��
                  SUCC            �ɹ�
                  FAIL            ʧ��
** ȫ�ֱ�������
** ����ģ�飺��
** ˵    ������Կ�ļ���ȻҲȡ��Ϊ"dl.tar.gz"����������Ϊ.tar.gz
             �ļ�������ͨ��֤�����ġ�
             ������ֻ��Ϊ��ͳһ���������Ĵ�����
****************************************************************/
int CreatePubkeyFile(const char *pIniFilename, const char *pFilename1,
                             const char *pFilename2, char *pKeyFilename)
{
	int hFileOut;
	char *pStrInfo = NULL;
	unsigned int uiKeyExpiredDate;

	//���ļ�
	if((hFileOut=open(pKeyFilename,O_RDWR|O_CREAT|O_TRUNC,0666))==FAIL){
		CFunCommon::DlSprintf("�ļ�pubkey��ʧ��");
		return FAIL;
	}

	//��ȡ��Կ��Ч��
	pStrInfo = GetInitKey(pIniFilename, "DownloadInfo", "uiKeyExpiredDate");
	if(pStrInfo == NULL) {
		CFunCommon::DlSprintf("Get uiKeyExpiredDate from INI failed.\n");
		close(hFileOut);
		return FAIL;
	}
	if(pStrInfo[0] == '\0') {
		CFunCommon::DlSprintf("The value of the \"%s\" is null. ", "uiKeyExpiredDate");
		close(hFileOut);
		return FAIL;
	}
	CFunCommon::DlSprintf("uiKeyExpiredDate=%s, strlen=%d\n", pStrInfo, strlen(pStrInfo));//test
	if(strlen(pStrInfo)!=6){
		CFunCommon::DlSprintf("KeyExpiredDate=%s, error!\n", pStrInfo);
		close(hFileOut);
		return FAIL;
	}
	uiKeyExpiredDate=strtoul(pStrInfo, NULL, 10);

	//���湫Կ��Ч��
	if(write(hFileOut, &uiKeyExpiredDate, sizeof(unsigned int))!=sizeof(unsigned int)){
		CFunCommon::DlSprintf("write uiKeyExpiredDate error!\n");
		close(hFileOut);
		return FAIL;
	}

	//����ģ��ָ��
	if(strcmp(pFilename1, "pub_mod") == 0){
		if(SavePubkeyModulus(pFilename1, hFileOut)== FAIL){
			close(hFileOut);
			return FAIL;
		}
		if(strcmp(pFilename2, "pub_exp") != 0){
			CFunCommon::DlSprintf("ERR: error for the name of file '%s'.\n", pFilename2);
			close(hFileOut);
			return FAIL;
		}
		if(SavePubkeyExponent(pFilename2, hFileOut)== FAIL){
			close(hFileOut);
			return FAIL;
		}
	}else if(strcmp(pFilename2, "pub_mod") == 0){
		if(SavePubkeyModulus(pFilename2, hFileOut)== FAIL){
			close(hFileOut);
			return FAIL;
		}
		if(strcmp(pFilename1, "pub_exp") != 0){
			CFunCommon::DlSprintf("ERR: error for the name of file '%s'.\n", pFilename2);
			close(hFileOut);
			return FAIL;
		}
		if(SavePubkeyExponent(pFilename1, hFileOut)== FAIL){
			close(hFileOut);
			return FAIL;
		}
	}else{
		CFunCommon::DlSprintf("ERR: error for the names of files.\n");
		close(hFileOut);
		return FAIL;
	}
	close(hFileOut);

	return SUCC;
}


/****************************************************************
** �������ܣ���˽Կģ�ļ��л�ȡ˽Կ��ģ�����浽��Կ�ṹ����
**
** ����������
                    pFilename        ��Կ�ļ���
                    pPrivateKey      ��Կ�ṹ��
** ����ֵ  ��
                    SUCC             �ɹ�
                    FAIL             ʧ��
** ȫ�ֱ�������
** ����ģ�飺��
** ˵    ����˽Կģ�ļ��е�������Ϣ����Ϊ˽Կ��ģ��
   ע��:
   ��Կ����Կ���ļ��о����ַ�����ʽ�洢��
   ��Կ��һ��4���ص�16�����������ļ��е�һ���ַ���
****************************************************************/
int SavePrikeyModulus(const char *pFilename, R_RSA_PUBLIC_KEY *pPrivateKey)
{
	struct stat stat_buf;
	int hFileIn;
	unsigned char *ptmp_mod=NULL, *p_mod=NULL;
	int i, j, temp, ch1;

	if(stat(pFilename, &stat_buf) == FAIL) {
		CFunCommon::DlSprintf("ERR: Get the size of file '%s' error.\n", pFilename);
		perror("stat");
		return FAIL;
	}
	CFunCommon::DlSprintf("\nThe size of file '%s' is %d\n", pFilename, (int)stat_buf.st_size);

	//save mod_bits
	pPrivateKey->bits=((stat_buf.st_size+1)/2)*8;//
	CFunCommon::DlSprintf("pPrivateKey->bits=%d\n", pPrivateKey->bits);

	if((hFileIn = open(pFilename, O_RDONLY)) == FAIL){
		CFunCommon::DlSprintf("ERR: cannot open in-file '%s'.\n", pFilename);
		return FAIL;
	}

	ptmp_mod=(unsigned char *)malloc(stat_buf.st_size);
	if(ptmp_mod==NULL){
		CFunCommon::DlSprintf("malloc for private mod error!\n");
		close(hFileIn);
		return FAIL;
	}

	//��ȡ˽Կ�ļ��е�ģ
	if(read(hFileIn,ptmp_mod,stat_buf.st_size)!=stat_buf.st_size){
		CFunCommon::DlSprintf("read private mod error!\n");
		close(hFileIn);
		free(ptmp_mod);
		return FAIL;
	}
	close(hFileIn);

	//���ַ�����ʽ��ģת��ΪBCD��
	p_mod=pPrivateKey->modulus+MAX_RSA_MODULUS_LEN-(stat_buf.st_size+1)/2;
	for(i=0,j=0; (i<stat_buf.st_size)&&(j<(stat_buf.st_size+1)/2); i++,j++)
	{
		if((i==0)&&(stat_buf.st_size%2 != 0))
		{//��Կ�ļ��к��������ַ�
			//��һ��ֻ��һ���ַ�
			if('A' <= ptmp_mod[i])/* A~F, a~f */
			{
				if('a' <= ptmp_mod[i])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_mod[i]-=32;
				}
				temp=ptmp_mod[i]-65+10;
				p_mod[j]=(0<<4)+((temp%16)&0x0F);
			}
			else
			{
				temp=ptmp_mod[i]-48;
				p_mod[j]=(0<<4)+((temp%10)&0x0F);
			}
		}
		else/*ÿ����2���ַ����һ��BCD��*/
		{
			if('A' <= ptmp_mod[i])/* A~F, a~f */
			{
				if('a' <= ptmp_mod[i])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_mod[i]-=32;
				}
				temp=ptmp_mod[i]-65+10;
				ch1=((temp%16)&0x0F)<<4;
			}
			else
			{
				temp=ptmp_mod[i]-48;
				ch1=((temp%10)&0x0F)<<4;
			}

			if('A' <= ptmp_mod[i+1])/* A~F, a~f */
			{
				if('a' <= ptmp_mod[i+1])/*Сд��ĸת��Ϊ��д��ĸ*/
				{
					ptmp_mod[i+1]-=32;
				}
				temp=ptmp_mod[i+1]-65+10;
				p_mod[j]=ch1+((temp%16)&0x0F);
			}
			else
			{
				temp=ptmp_mod[i+1]-48;
				p_mod[j]=ch1+((temp%10)&0x0F);
			}

			i++;/*add 2*/
		}
	}
	free(ptmp_mod);

	return SUCC;
}


/****************************************************************
** �������ܣ���˽Կָ���ļ��л�ȡ˽Կ��ָ�������浽��Կ�ṹ����
**
** ����������
                    pFilename        ��Կ�ļ���
                    pPrivateKey      ��Կ�ṹ��
** ����ֵ  ��
                    SUCC             �ɹ�
                    FAIL             ʧ��
** ȫ�ֱ�������
** ����ģ�飺��
** ˵    ����˽Կָ���ļ��е�������Ϣ����Ϊ˽Կ��ָ����
****************************************************************/
int SavePrivkeyExponent(const char *pFilename, R_RSA_PUBLIC_KEY *pPrivateKey)
{
	struct stat stat_buf;
	int hFileIn;
	unsigned char *ptmp_exp=NULL, *p_exp=NULL;
	int i, j, temp, ch1;

	if(stat(pFilename, &stat_buf) == FAIL) 
	{
		return FAIL;
	}

	if((hFileIn = open(pFilename, O_RDONLY)) == FAIL)
	{
		return FAIL;
	}

	ptmp_exp=(unsigned char *)malloc(stat_buf.st_size);
	if(ptmp_exp==NULL)
	{
		close(hFileIn);
		return FAIL;
	}

	//save exponent
	if(read(hFileIn,ptmp_exp,stat_buf.st_size)!=stat_buf.st_size)
	{
		close(hFileIn);
		free(ptmp_exp);
		return FAIL;
	}
	close(hFileIn);

	//���ַ�����ʽ��ģת��ΪBCD��
	p_exp=pPrivateKey->exponent+MAX_RSA_MODULUS_LEN-(stat_buf.st_size+1)/2;
	for(i=0,j=0; (i<stat_buf.st_size)&&(j<(stat_buf.st_size+1)/2); i++,j++){
		if((i==0)&&(stat_buf.st_size%2 != 0)){//��Կ�ļ��к��������ַ�
			//��һ��ֻ��һ���ַ�
			if('A' <= ptmp_exp[i]){/* A~F, a~f */
				if('a' <= ptmp_exp[i]){/*Сд��ĸת��Ϊ��д��ĸ*/
					ptmp_exp[i]-=32;
				}
				temp=ptmp_exp[i]-65+10;
				p_exp[j]=(0<<4)+((temp%16)&0x0F);
			}else{
				temp=ptmp_exp[i]-48;
				p_exp[j]=(0<<4)+((temp%10)&0x0F);
			}
		}else{/*ÿ����2���ַ����һ��BCD��*/
			if('A' <= ptmp_exp[i]){/* A~F, a~f */
				if('a' <= ptmp_exp[i]){/*Сд��ĸת��Ϊ��д��ĸ*/
					ptmp_exp[i]-=32;
				}
				temp=ptmp_exp[i]-65+10;
				ch1=((temp%16)&0x0F)<<4;
			}else{
				temp=ptmp_exp[i]-48;
				ch1=((temp%10)&0x0F)<<4;
			}

			if('A' <= ptmp_exp[i+1]){/* A~F, a~f */
				if('a' <= ptmp_exp[i+1]){/*Сд��ĸת��Ϊ��д��ĸ*/
					ptmp_exp[i+1]-=32;
				}
				temp=ptmp_exp[i+1]-65+10;
				p_exp[j]=ch1+((temp%16)&0x0F);
			}else{
				temp=ptmp_exp[i+1]-48;
				p_exp[j]=ch1+((temp%10)&0x0F);
			}
			i++;
		}
	}
	free(ptmp_exp);

#ifdef DEBUG_PACK
	CFunCommon::DlSprintf("pPrivateKey->exp_len=%d\n", (int)((stat_buf.st_size+1)/2));
	CFunCommon::DlSprintf("pPrivateKey->exponent=\n");
	for(j=0; j<(stat_buf.st_size+1)/2; j++)
		CFunCommon::DlSprintf("%x.", p_exp[j]);
	CFunCommon::DlSprintf("\n");
#endif

	return SUCC;
}






#endif
/*
	int hFile_in;
	CString strSavePath;
	int nBytesRead, nBlockRead, nFileRead = 0;
	int iret;
	unsigned char unit_inbuf_tmp[UNIT_BUF_SIZE], unit_inbuf[UNIT_BUF_SIZE];
	unsigned char unit_outbuf[UNIT_BUF_SIZE];
	unsigned char des_inbuf[TDES_INPUT_SIZE], des_outbuf[TDES_INPUT_SIZE];
	unsigned char xor_inbuf_pre[TDES_INPUT_SIZE];
	unsigned char *punit_inbuf_tmp = NULL, *punit_inbuf = NULL;
	unsigned char *punit_outbuf = NULL;
	unsigned char key[16] = {1, 3, 5, 7, 2, 4, 6, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	int i, j;
	int bFileStart = TRUE, bFileEnd = FALSE;
	unsigned int iHeaderLen = 0, iNldFileLen;
	struct stat stat_buf;
	DATA1 chartoint;

	NLD_HEADERINFO *pHeaderInfo;
	pHeaderInfo=(NLD_HEADERINFO *)malloc(sizeof(NLD_HEADERINFO));

	memset(pHeaderInfo,0,sizeof(NLD_HEADERINFO));

	if (stat(pInFilename, &stat_buf) == FAIL) 
	{
		return FAIL;
	}
	iNldFileLen = stat_buf.st_size;

	if ((hFile_in = open(pInFilename, O_RDONLY)) == FAIL) 
	{
		return FAIL;
	}

	//�����㷨��ʼ
	do {
		// 1.�ļ��ָ��2048�ֽڴ�С�����ݿ飬��ȡһ�����ݿ�
		punit_inbuf_tmp = unit_inbuf_tmp;
		punit_inbuf = unit_inbuf;
		nBlockRead = 0;
		while (nBlockRead < sizeof(unit_inbuf)) 
		{ //ѭ����ȡ2048�ֽ�����//
			while (((nBytesRead=read(hFile_in,punit_inbuf_tmp,sizeof(unit_inbuf)-nBlockRead))==-1)&&(errno==EINTR))
				;
			if (nBytesRead < 0) 
			{
				close(hFile_in);
				return FAIL;
			} 
			else if (nBytesRead == 0) 
			{ // read file end.
				bFileEnd = TRUE;
				break;
			} 
			else if (nBytesRead < (sizeof(unit_inbuf)-nBlockRead)) 
			{
				memcpy(punit_inbuf, punit_inbuf_tmp, nBytesRead);
				punit_inbuf_tmp += nBytesRead;
				punit_inbuf += nBytesRead;
			} 
			else 
			{
				memcpy(punit_inbuf, punit_inbuf_tmp, nBytesRead);
			}
			nBlockRead += nBytesRead;
		}

		if (bFileEnd == TRUE) 
		{ 
			if ((nBlockRead != 0)&&(nBlockRead != sizeof(unit_inbuf))) 
			{
				close(hFile_in);
				return FAIL;
			} 
			else 
				if (nBlockRead == 0)//the last block of file is null.
					break; 
		} 
		else 
		{
			if (nBlockRead != sizeof(unit_inbuf)) 
			{

				close(hFile_in);
				return FAIL;
			}
		}
		nFileRead += nBlockRead;

		// 2.����һ�����ݿ�(2048�ֽ�)
		punit_inbuf = unit_inbuf;
		punit_outbuf = unit_outbuf;
		for (i=0; i<(sizeof(unit_inbuf)/sizeof(des_inbuf)); i++) 
		{
			memcpy(des_inbuf, punit_inbuf, sizeof(des_inbuf)); //��ʼ��des_inbuf[]
			punit_inbuf += sizeof(des_inbuf); //�ƶ�punit_inbufָ��

			if (i == 0) 
			{ //��8���ֽڲ���TDES�㷨����
				iret = mdes3(DISCRYPT, des_inbuf, des_outbuf, key);
				if (iret == FAIL) 
				{
					close(hFile_in);
					return FAIL;
				}
			} 
			else 
			{ //����ÿ8���ֽڲ�����ǰ8���ֽڵļ��ܽ������XOR 
				for (j=0; j<sizeof(des_inbuf); j++) 
				{
					des_outbuf[j] = xor_inbuf_pre[j] ^ des_inbuf[j];
				}
			}

			memcpy(punit_outbuf, des_outbuf, sizeof(des_outbuf));
			punit_outbuf += sizeof(des_outbuf); //�ƶ�punit_outbufָ��
			memcpy(xor_inbuf_pre, des_inbuf, sizeof(des_inbuf));//����xor_inbuf_pre[]
		}

		// 3.����ǵ�һ�����ݿ飬��ȡͷ����Ϣ�ṹ
		if (bFileStart == TRUE) 
		{
			memcpy(chartoint.c,unit_outbuf+HEADINFO_SIZE_OFF,sizeof(unsigned int));
			iHeaderLen=sizeof(NLD_HEADERINFO);
			iHeaderLen = chartoint.i;

			if (iHeaderLen != sizeof(NLD_HEADERINFO)) 
			{
				close(hFile_in);
				return FAIL;
			}
			memcpy(pHeaderInfo, unit_outbuf, iHeaderLen);
		}
		// 4.��������һ�����ݿ飬ȥ������ַ�
		if (nFileRead == iNldFileLen) 
		{
			nBlockRead = nBlockRead-(iNldFileLen-iHeaderLen-
				pHeaderInfo->uiTarFileLen-pHeaderInfo->uiSignatureLen);
		}

		bFileStart = FALSE;
	} while (1);
	
	close(hFile_in);

	CString temp;
	iniHead.SetName("Headerinfo.ini");
	iniHead.SetPath(g_szAppPath+"unnzp\\");

	iniHead.OpenIniFileForWrite();

	iniHead.WriteSection("DownloadInfo");
	iniHead.WriteItemString("strMachineType",pHeaderInfo->cMachineType);
	iniHead.WriteItemString("cType","P");
	temp=_T("");
	temp.Format("%d",pHeaderInfo->uiKeyExpiredDate);
	iniHead.WriteItemString("uiKeyExpiredDate",temp);

	iniHead.WriteSection("AppInfo");
	iniHead.WriteItemString("strAppName",pHeaderInfo->UserPrgInfo.cAppName);
	iniHead.WriteItemString("strVerBuf",pHeaderInfo->UserPrgInfo.cVerBuf);
	temp=_T("");
	temp.Format("%d",pHeaderInfo->UserPrgInfo.cSeriNo);
	iniHead.WriteItemString("iSeriNo",temp);
	iniHead.WriteItemString("strBuildTime",pHeaderInfo->UserPrgInfo.tBuildTime);
	temp=_T("");
	temp.Format("%d",pHeaderInfo->UserPrgInfo.bIsMaster);
	iniHead.WriteItemString("bIsMaster",temp);
	temp=_T("");
	temp.Format("%d,%d,%d",pHeaderInfo->UserPrgInfo.sReverse[0],
		pHeaderInfo->UserPrgInfo.sReverse[1],pHeaderInfo->UserPrgInfo.sReverse[2]);
	iniHead.WriteItemString("sReverse",temp);
	
	iniHead.WriteSection("Extended");
	iniHead.WriteItemString("strExtended",pHeaderInfo->cExtended);

	iniHead.CloseIniFile();

	free(pHeaderInfo);

	return SUCC;
*/
