#include <Windows.h>
#include "CNetPacket.h"

MemoryPoolTLS<CNetPacket> CNetPacket::_packetpool(50000, POOL_MAX_ALLOC, false);

CNetPacket* CNetPacket::Alloc()
{
	CNetPacket* pPacket = _packetpool.Alloc();
	pPacket->AddRefCount();
	return pPacket;
}

void CNetPacket::Free(CNetPacket* pPacket)
{
	if (InterlockedDecrement((LONG*)&pPacket->_refCount) == 0)
	{
		pPacket->Clear();
		_packetpool.Free(pPacket);
	}
}

CNetPacket::CNetPacket() :_refCount(0), _cBufferSize(DEFAULT_SIZE), _isEncode(false)
{
	_pBuffer = new char[DEFAULT_SIZE];
	// Code(1byte) + Len(2byte) + RandKey(1byte) + CheckSum(1byte) = 5byte
	_pReadPos = _pWritePos = _pBuffer + sizeof(NetHeader);
	srand((UINT)time(NULL));
}

CNetPacket::CNetPacket(int size) :_refCount(0), _cBufferSize(size), _isEncode(false)
{
	_pBuffer = new char[size];
	_pReadPos = _pWritePos = _pBuffer + sizeof(NetHeader);
	srand((UINT)time(NULL));
}

CNetPacket::~CNetPacket()
{
	delete[] _pBuffer;
}

//////////////////////////////////////////////////////////////////////////
// ��Ŷ  �ı�.
//
// Parameters: ����.
// Return: ����.
//////////////////////////////////////////////////////////////////////////
void CNetPacket::Release(void)
{
	this->~CNetPacket();
}

//////////////////////////////////////////////////////////////////////////
// ��Ŷ û��.
//
// Parameters: ����.
// Return: ����.
//////////////////////////////////////////////////////////////////////////
void CNetPacket::Clear(void)
{
	_pWritePos = _pReadPos = _pBuffer + sizeof(NetHeader);
	_isEncode = false;
}

//////////////////////////////////////////////////////////////////////////
// ���� ������ ���.
//
// Parameters: ����.
// Return: (int)��Ŷ ���� ������ ���.
//////////////////////////////////////////////////////////////////////////
int	CNetPacket::GetBufferSize(void)
{
	return _cBufferSize;
}

//////////////////////////////////////////////////////////////////////////
// ���� ������� ������ ���.
//
// Parameters: ����.
// Return: (int)������� ����Ÿ ������.
//////////////////////////////////////////////////////////////////////////
int CNetPacket::GetPayloadSize(void)
{
	return _pWritePos - _pReadPos;
}

int	CNetPacket::GetTotalSize(void)
{
	return _pWritePos - _pBuffer;
}

//////////////////////////////////////////////////////////////////////////
// ���� ������ ���.
//
// Parameters: ����.
// Return: (char *)���� ������.
//////////////////////////////////////////////////////////////////////////
char* CNetPacket::GetBufferPtr(void)
{
	return _pBuffer;
}

char* CNetPacket::GetPayloadPtr(void)
{
	return _pBuffer + sizeof(NetHeader);
}

//////////////////////////////////////////////////////////////////////////
// ���� Pos �̵�. (�����̵��� �ȵ�)
// GetBufferPtr �Լ��� �̿��Ͽ� �ܺο��� ������ ���� ������ ������ ��� ���. 
//
// Parameters: (int) �̵� ������.
// Return: (int) �̵��� ������.
//////////////////////////////////////////////////////////////////////////
int CNetPacket::MoveWritePos(int size)
{
	int remainSize = _cBufferSize - GetTotalSize();

	if (size > remainSize)
		size = remainSize;

	_pWritePos += size;
	return size;
}

int CNetPacket::MoveReadPos(int size)
{
	int remainSize = _pWritePos - _pReadPos;

	if (size > remainSize)
		size = remainSize;

	_pReadPos += size;
	return size;
}

/* ============================================================================= */
// ������ �����ε�
/* ============================================================================= */

//////////////////////////////////////////////////////////////////////////
// �ֱ�.	�� ���� Ÿ�Ը��� ��� ����.
//////////////////////////////////////////////////////////////////////////
CNetPacket& CNetPacket::operator<< (char val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}
CNetPacket& CNetPacket::operator<< (unsigned char val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}

CNetPacket& CNetPacket::operator<< (short val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}
CNetPacket& CNetPacket::operator<< (unsigned short val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}

CNetPacket& CNetPacket::operator<< (int val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}
CNetPacket& CNetPacket::operator<< (unsigned int val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}

CNetPacket& CNetPacket::operator<< (long val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}
CNetPacket& CNetPacket::operator<< (unsigned long val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}

CNetPacket& CNetPacket::operator<< (__int64 val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}
CNetPacket& CNetPacket::operator<< (unsigned __int64 val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}

CNetPacket& CNetPacket::operator<< (float val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}
CNetPacket& CNetPacket::operator<< (double val)
{
	SetData((char*)&val, sizeof(val));
	return *this;
}


//////////////////////////////////////////////////////////////////////////
// ����.	�� ���� Ÿ�Ը��� ��� ����.
//////////////////////////////////////////////////////////////////////////
CNetPacket& CNetPacket::operator>> (char& chValue)
{
	GetData((char*)&chValue, sizeof(chValue));
	return *this;
}
CNetPacket& CNetPacket::operator>> (unsigned char& byValue)
{
	GetData((char*)&byValue, sizeof(byValue));
	return *this;
}

CNetPacket& CNetPacket::operator>> (short& shValue)
{
	GetData((char*)&shValue, sizeof(shValue));
	return *this;
}
CNetPacket& CNetPacket::operator>> (unsigned short& wValue)
{
	GetData((char*)&wValue, sizeof(wValue));
	return *this;
}

CNetPacket& CNetPacket::operator>> (int& iValue)
{
	GetData((char*)&iValue, sizeof(iValue));
	return *this;
}
CNetPacket& CNetPacket::operator>> (unsigned int& uiValue)
{
	GetData((char*)&uiValue, sizeof(uiValue));
	return *this;
}

CNetPacket& CNetPacket::operator>> (long lValue)
{
	GetData((char*)&lValue, sizeof(lValue));
	return *this;
}
CNetPacket& CNetPacket::operator>> (unsigned long& ulValue)
{
	GetData((char*)&ulValue, sizeof(ulValue));
	return *this;
}

CNetPacket& CNetPacket::operator>> (__int64& iValue)
{
	GetData((char*)&iValue, sizeof(iValue));
	return *this;
}
CNetPacket& CNetPacket::operator>> (unsigned __int64& uiValue)
{
	GetData((char*)&uiValue, sizeof(uiValue));
	return *this;
}

CNetPacket& CNetPacket::operator>> (float& fValue)
{
	GetData((char*)&fValue, sizeof(fValue));
	return *this;
}
CNetPacket& CNetPacket::operator>> (double& dValue)
{
	GetData((char*)&dValue, sizeof(dValue));
	return *this;
}



//////////////////////////////////////////////////////////////////////////
// ����Ÿ ���.
//
// Parameters: (char *)Dest ������. (int)Size.
// Return: (int)������ ������.
//////////////////////////////////////////////////////////////////////////
int CNetPacket::GetData(char* pDest, int size)
{
	int remainSize = _pWritePos - _pReadPos;
	if (size > remainSize)
		size = remainSize;

	memcpy_s(pDest, size, _pReadPos, size);
	_pReadPos += size;
	return size;
}

//////////////////////////////////////////////////////////////////////////
// ����Ÿ ����.
//
// Parameters: (char *)Src ������. (int)SrcSize.
// Return: (int)������ ������.
//////////////////////////////////////////////////////////////////////////
int CNetPacket::SetData(char* pSrc, int srcSize)
{
	int remainSize = _cBufferSize - (_pWritePos - _pBuffer);

	if (srcSize > remainSize)
		srcSize = remainSize;

	memcpy_s(_pWritePos, srcSize, pSrc, srcSize);
	_pWritePos += srcSize;
	return srcSize;
}

void CNetPacket::GetNetHeader(char* pDest)
{
	memcpy_s(pDest, sizeof(NetHeader), _pBuffer, sizeof(NetHeader));
}

void CNetPacket::SetNetHeader(char* pSrc)
{
	memcpy_s(_pBuffer, sizeof(NetHeader), pSrc, sizeof(NetHeader));
}

// Ref Count
void CNetPacket::AddRefCount()
{
	InterlockedIncrement((LONG*)&_refCount);
}

bool CNetPacket::Encode()
{
	if (_isEncode)
		return false;

	_isEncode = true;

	// Set Network Header
	NetHeader header;
	header.code = dfPACKET_CODE;
	header.len = GetPayloadSize();
	header.randKey = rand() % 256;
	PCHAR pPayload = GetPayloadPtr();

	INT64 sumVal = 0;
	for (int i = 0; i < header.len; i++)
		sumVal += pPayload[i];

	header.chkSum = sumVal % 256;
	SetNetHeader((PCHAR)&header);

	// Encode checksum & payload
	CHAR befRk = 0;
	CHAR befPk = 0;
	INT cnt = 1;
	PCHAR pEncode = pPayload - sizeof(BYTE);
	for (INT i = 0; i < header.len + sizeof(BYTE); i++)
	{
		befRk = pEncode[i] ^ (befRk + header.randKey + cnt);
		befPk = pEncode[i] = befRk ^ (befPk + dfPACKET_KEY + cnt);
		++cnt;
	}
}

bool CNetPacket::Decode()
{
	NetHeader header;
	GetNetHeader((PCHAR)&header);

	// Decode checksum & payload
	PCHAR pDecode = GetPayloadPtr() - sizeof(BYTE);
	INT cnt = 1;
	CHAR befPk = 0;
	CHAR befRk = 0;
	CHAR tmp = 0;
	USHORT len = header.len;

	for (INT i = 0; i < len + sizeof(BYTE); i++)
	{
		tmp = pDecode[i] ^ (befRk + dfPACKET_KEY + cnt);
		befRk = pDecode[i];

		pDecode[i] = tmp ^ (befPk + header.randKey + cnt);
		befPk = tmp;
		++cnt;
	}

	// Compare checksum
	GetNetHeader((PCHAR)&header);
	PCHAR pPayload = GetPayloadPtr();
	INT64 sumVal = 0;
	BYTE myChksum;
	for (int i = 0; i < header.len; i++)
		sumVal += pPayload[i];

	myChksum = sumVal % 256;

	if (header.chkSum != myChksum)
		return false;
	else
		return true;
}