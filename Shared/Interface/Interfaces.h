﻿#ifndef IData_h__20160825
#define IData_h__20160825

#pragma once
#ifndef OUT
#define OUT
#define IN
#endif

#include <complex>
#include "smartptr.h"

namespace Yap
{
	const int DataTypeUnknown		= 0x00000000;	///< enum not initialized yet.
	const int DataTypeChar			= 0x00000001;	///< wchar_t (1)
	const int DataTypeUnsignedChar	= 0x00000002;	///< unsigned wchar_t (1)
	const int DataTypeShort			= 0x00000004;	///< short (2)
	const int DataTypeUnsignedShort	= 0x00000008;	///< unsigned short (2)
	const int DataTypeFloat			= 0x00000010;	///< float (4)
	const int DataTypeDouble		= 0x00000020;	///< double (8)
	const int DataTypeInt			= 0x00000040;	///< int (4)
	const int DataTypeUnsignedInt	= 0x00000080;	///< unsigned int (4)
	const int DataTypeComplexFloat	= 0x00000100;	///< complex<float> (8)
	const int DataTypeComplexDouble	= 0x00000200;	///< complex<double> (16)
	const int DataTypeBool			= 0x00000400;	///< bool
	const int DataTypeAll			= 0xFFFFFFFF;	///< 若为此类型，说明此处理器对什么类型都接受。

	template<typename T> struct data_type_id
	{
		static const int type;
	};

	template <> struct data_type_id<double> { static const int type = DataTypeDouble; };
	template <> struct data_type_id<char> { static const int type = DataTypeChar; };
	template <> struct data_type_id<unsigned char> { static const int type = DataTypeUnsignedChar; };
	template <> struct data_type_id<short> { static const int type = DataTypeShort; };
	template <> struct data_type_id <unsigned short> { static const int type = DataTypeUnsignedShort; };
	template <> struct data_type_id <float> { static const int type = DataTypeFloat; };
	template <> struct data_type_id <unsigned int> { static const int type = DataTypeUnsignedInt; };
	template <> struct data_type_id <int> { static const int type = DataTypeInt; };
	template <> struct data_type_id <std::complex<float>> { static const int type = DataTypeComplexFloat; };
	template <> struct data_type_id <std::complex<double>> { static const int type = DataTypeComplexFloat; };
	template <> struct data_type_id <bool> { static const int type = DataTypeBool; };


	/// 定义了磁共振图像数据中的常用的维度。
	/**
	通常在定义多维数据时，Readout, PhaseEncoding, Slice三个方向会优先连续存储。
	其他维度的次序则不确定。习惯上在描述数据维度信息时，应按照习惯的名称命名。
	*/
	enum DimensionType
	{
		DimensionReadout,
		DimensionPhaseEncoding,
		DimensionSlice,
		Dimension4,				// 为了兼容性考虑，意义不明确
		DimensionChannel,
		DimensionAverage,
		DimensionSlab,
		DimensionEcho,
		DimensionPhase,

		DimensionUser1,
		DimensionUser2,
		DimensionUser3,
		DimensionUser4,
		DimensionUser5,
		DimensionUser6,
		DimensionInvalid = 100,
	};

	const unsigned int YAP_ANY_DIMENSION = 0xffffffff;

	struct Dimension
	{
		DimensionType type;
		unsigned int start_index;
		unsigned int length;
		Dimension();
		Dimension(DimensionType type, unsigned start_index, unsigned int length);
	};

	struct IDimensions : public ISharedObject
	{
		virtual unsigned int GetDimensionCount() = 0;
		virtual bool GetDimensionInfo(unsigned int index, OUT DimensionType& dimension_type,
			OUT unsigned int& start_index, OUT unsigned int& length) = 0;
	};

	struct IGeometry
	{
		virtual void GetSpacing(double& x, double& y, double& z) = 0;
		virtual void GetRowVector(double& x, double& y, double& z) = 0;
		virtual void GetColumnVector(double& x, double& y, double& z) = 0;
		virtual void GetSliceVector(double& x, double& y, double& z) = 0;
		virtual void GetReadoutVector(double& x, double& y, double& z) = 0;
		virtual void GetPhaseEncodingVector(double& x, double& y, double& z) = 0;
		virtual void GetCenter(double& x, double& y, double& z) = 0;

		virtual void SetSpacing(double x, double y, double z) = 0;
		virtual void SetRowVector(double x, double y, double z) = 0;
		virtual void SetColumnVector(double x, double y, double z) = 0;
		virtual void SetSliceVector(double x, double y, double z) = 0;
		virtual void SetReadoutVector(double x, double y, double z) = 0;
		virtual void SetPhaseEncodingVector(double x, double y, double z) = 0;
		virtual void SetCenter(double x, double y, double z) = 0;

		virtual bool IsValid() = 0;
	};

	struct IData : public ISharedObject
	{
		virtual int GetDataType() = 0;				///< 返回数据元素的类型
		virtual IDimensions * GetDimensions() = 0;	///< 获得数据的维度信息。
	};

	template <typename T> struct IDataArray : public IData
	{
		/// 返回数组的起始地址。
		virtual T * GetData() = 0;
	};

	template <typename ELEMENT_TYPE>
	struct IIterator
	{
		virtual ELEMENT_TYPE * GetFirst() = 0;
		virtual ELEMENT_TYPE * GetNext() = 0;
	};

	template <typename ELEMENT_TYPE>
	struct IContainer : public ISharedObject
	{
		typedef IIterator<ELEMENT_TYPE> iterator;

		virtual ELEMENT_TYPE * Find(const wchar_t * name) = 0;
		virtual iterator * GetIterator() = 0;
		virtual bool Add(const wchar_t * name, ELEMENT_TYPE * element) = 0;
	};

	struct IProcessor;
	typedef IContainer<IProcessor> IProcessorContainer;
	typedef IProcessorContainer::iterator IProcessorIter;

	enum PropertyType
	{
		PropertyBool,
		PropertyInt,
		PropertyFloat,
		PropertyString,
		PropertyStruct,
		PropertyBoolArray,
		PropertyIntArray,
		PropertyFloatArray,
		PropertyStringArray,
		PropertyStructArray,
	};

	template<typename T> struct property_type_id
	{
		static const int type;
	};

	template <> struct property_type_id<bool> { static const int type = PropertyBool; };
	template <> struct property_type_id<int> { static const int type = PropertyInt; };
	template <> struct property_type_id<double> { static const int type = PropertyFloat; };
	template <> struct property_type_id<std::wstring> { static const int type = PropertyString; };


	struct IProperty : public ISharedObject
	{
		virtual PropertyType GetType() const = 0;
		virtual const wchar_t * GetName() const = 0;
        virtual void SetName(const wchar_t * name) = 0;
		virtual const wchar_t * GetDescription() const = 0;
        virtual void SetDescription(const wchar_t * description) = 0;
		/// Returns pointer to one of the value interfaces.
		/**
		\return Returned pointer can be reinterpret_cast to a 'value interface' corresponding 
		to the type of the variable, such as IDoubleValue, IIntValue, IBoolValue, IStringValue.
		*/
		virtual void * ValueInterface() = 0;
	};
	typedef IContainer<IProperty> IPropertyContainer;
	typedef IPropertyContainer::iterator IPropertyIter;

	template <typename VALUE_TYPE>
	struct IValue
	{
		virtual VALUE_TYPE Get() const = 0;
		virtual void Set(const VALUE_TYPE value) = 0;
	};
	typedef IValue<double> IDoubleValue;
	typedef IValue<int> IIntValue;
	typedef IValue<bool> IBoolValue;

	struct IStringValue
	{
		virtual const wchar_t* Get() const = 0;
		virtual void Set(const wchar_t* value) = 0;
	};

    typedef IContainer<IProperty> IStructValue;

	template <typename VALUE_TYPE>
	struct IArrayValue
	{
		virtual size_t GetSize() const = 0;
		virtual void SetSize(size_t size) = 0;
		virtual VALUE_TYPE * Elements() = 0;
	};

	struct IPort : public ISharedObject
	{
		virtual const wchar_t * GetName() const = 0;
		virtual unsigned int GetDimensionCount() const = 0;
		virtual int GetDataType() const = 0;
	};

	typedef IContainer<IPort> IPortContainer;
	typedef IPortContainer::iterator IPortIter;


	struct IProcessor : public ISharedObject
	{
		virtual IProcessor * Clone() const override = 0;
		virtual const wchar_t * GetClassId() const = 0;
		virtual void SetClassId(const wchar_t * id) = 0;

		virtual const wchar_t * GetInstanceId() const = 0;
		virtual void SetInstanceId(const wchar_t * instance_id) = 0;

		virtual IPortContainer * Inputs() = 0;
		virtual IPortContainer * Outputs() = 0;

		/// 获得属性访问接口。
		virtual IPropertyContainer * GetProperties() = 0;

		/// 将指定名称的属性与参数空间的参数相关联。
		virtual bool LinkProperty(const wchar_t * property_id, const wchar_t * param_id) = 0;

		/// 接口用户调用这个函数来通知模块利用参数空间中的参数更新属性。
		virtual bool UpdateProperties(IPropertyContainer * params) = 0;

		/// 将指定处理模块的输入端口链接到当前模块指定的输出端口上。
		virtual bool Link(const wchar_t * output, IProcessor * next, const wchar_t * next_input) = 0;

		/// 向当前处理模块馈送数据。
		virtual bool Input(const wchar_t * name, IData * data) = 0;
	};

	typedef IContainer<IProcessor> IProcessorContainer;
	typedef IProcessorContainer::iterator IProcessorIter;
}
#endif // IData_h__
