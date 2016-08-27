#pragma once

#ifndef IProcessor_h__20160818
#define IProcessor_h__20160818

#include "IMemory.h"

namespace Yap
{
	struct IData;
	struct IParam;
	struct IPropertyEnumerator;

	struct IPort
	{
		virtual const wchar_t * GetName() = 0;
		virtual unsigned int GetDimensions() = 0;
		virtual int GetDataType() = 0;
	};

	struct IPortEnumerator
	{
		virtual IPort * GetFirstPort() = 0;
		virtual IPort * GetNextPort() = 0;
		virtual IPort * GetPort(const wchar_t * name) = 0;
	};

	struct IProcessor 
	{
		/// Return a cloned copy of this object.
		virtual IProcessor * Clone() = 0;

		/// Delete this object.
		virtual void Delete() = 0;

		virtual const wchar_t * GetClassId() = 0;
		virtual void SetClassId(const wchar_t * id) = 0;

		virtual const wchar_t * GetInstanceId() = 0;
		virtual void SetInstanceId(const wchar_t * instance_id) = 0;

		virtual IPortEnumerator * GetInputPortEnumerator() = 0;
		virtual IPortEnumerator * GetOutputPortEnumerator() = 0;

		/// ������Է��ʽӿڡ�
		virtual IPropertyEnumerator * GetProperties() = 0;

		/// ��ָ�����Ƶ�����������ռ�Ĳ����������
		virtual bool LinkProperty(const wchar_t * property_id, const wchar_t * param_id) = 0;

		/// �ӿ��û��������������֪ͨģ�����ò����ռ��еĲ����������ԡ�
		virtual bool UpdateProperties(IPropertyEnumerator * params) = 0;

		/// ��ָ������ģ�������˿����ӵ���ǰģ��ָ��������˿��ϡ�
		virtual bool Link(const wchar_t * output, IProcessor * next, const wchar_t * next_input) = 0;

		/// ��ǰ����ģ���������ݡ�
		virtual bool Input(const wchar_t * name, IData * data) = 0;
	};

	struct IProcessorManager
	{
		virtual IProcessor * GetFirstProcessor() = 0;
		virtual IProcessor * GetNextProcessor() = 0;
		virtual IProcessor * GetProcessor(const wchar_t * name) = 0;
	};

}


#endif // IProcessor_h__