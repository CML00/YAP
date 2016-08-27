#include "DataImp.h"

using namespace Yap;

/**
	����������ʹ�õ��ܵ�ά����
*/
unsigned int Yap::CDimensionsImp::GetDimensionCount() 
{
	return static_cast<unsigned int>(_dimension_info.size());
}

/**
	���ָ����ά�ȵ���Ϣ��
	@param  dimension_index ָ��ϣ���������Ϣ��ά�ȵ���������0��ʼ��
	@param  dimension_type ���������ָ�����������ͣ����壩��
	@param  start_index ��ص����������ά���ϵ���ʼλ�á�
	@param  length ��ص����������ά���ϵĿ�ȡ�
	@return
*/
bool Yap::CDimensionsImp::GetDimensionInfo(unsigned int dimension_index, 
										   DimensionType & dimension_type, 
	unsigned int& start_index, 
												  unsigned int& length)
{
	assert(dimension_index < _dimension_info.size());
	if (dimension_index >= _dimension_info.size())
	{
		return false;
	}

	dimension_type = _dimension_info[dimension_index].type;
	start_index = _dimension_info[dimension_index].start_index;
	length = _dimension_info[dimension_index].length;

	return true;
}

IClonable * Yap::CDimensionsImp::Clone()
{
	try
	{
		return new CDimensionsImp(*this);
	}
	catch (std::bad_alloc&)
	{
		return nullptr;
	}
}

void Yap::CDimensionsImp::Delete()
{
	throw std::logic_error("The method or operation is not implemented.");
}

CDimensionsImp & Yap::CDimensionsImp::operator()(DimensionType type, unsigned int index, unsigned int length)
{
	assert(type != DimensionInvalid);

	_dimension_info.push_back(Dimension(type, index, length));

	return *this;
}

Yap::CDimensionsImp::CDimensionsImp()
{
}

Yap::CDimensionsImp::CDimensionsImp(const CDimensionsImp& source)
{
	_dimension_info = source._dimension_info;
}

Yap::CDimensionsImp::CDimensionsImp(IDimensions * source)
{
	DimensionType type;
	unsigned int index, length;

	for (unsigned int i = 0; i < source->GetDimensionCount(); ++i)
	{
		source->GetDimensionInfo(i, type, index, length);
		_dimension_info.push_back(Dimension(type, index, length));
	}
}

Yap::CDimensionsImp::CDimensionsImp(unsigned int dimension_count)
{
	assert(dimension_count < 16);
	_dimension_info.resize(dimension_count);
}

bool Yap::CDimensionsImp::SetDimensionInfo(unsigned int dimension_index, 
												  DimensionType dimension_type, 
												  unsigned int start_index, 
												  unsigned int length)
{
	assert(dimension_index < _dimension_info.size());
	_dimension_info[dimension_index].type = dimension_type; //type�����Ӧ����modify

	_dimension_info[dimension_index].start_index = start_index;
	_dimension_info[dimension_index].length = length;

	return true;
}

bool Yap::CDimensionsImp::ModifyDimension(DimensionType type, 
												 unsigned int length,
												 unsigned int start_index)
{
	for (unsigned int i = 0; i < _dimension_info.size(); ++i)
	{
		if (_dimension_info[i].type == type)
		{
			_dimension_info[i].length = length;
			_dimension_info[i].start_index = start_index;
			return true;
		}
	}

	return false;
}

unsigned int Yap::CDimensionsImp::GetLength(unsigned int dimension_index)
{
	assert(dimension_index < _dimension_info.size());

	return _dimension_info[dimension_index].length;
}

Yap::Dimension::Dimension(DimensionType type_, 
	unsigned start_index_, 
	unsigned int length_) :
	type(type_), start_index(start_index_), length(length_)
{
}

Yap::Dimension::Dimension() :
	start_index(0),
	length(0),
	type(DimensionInvalid)
{
}
