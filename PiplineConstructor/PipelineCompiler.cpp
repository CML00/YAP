#include "stdafx.h"
#include "PipelineCompiler.h"
#include <cassert>
#include <boost/assign/list_of.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../Utilities/macros.h"
#include "PipelineConstructor.h"
#include <map>
#include <ctype.h>

using namespace Yap;
using namespace std;

void CStatement::Reset()
{
	_type = StatementUnknown;
	_tokens.clear();
}

void CStatement::Begin()
{
	_iter = _tokens.begin();
}

map<TokenType, wstring> token_map = boost::assign::map_list_of
	(TokenOperatorPointer, L"->")
	(TokenOperatorLink, L"<=>")
	(TokenOperatorDot, L".")
	(TokenOperatorAssign, L"=")
	(TokenComma, L",")
	(TokenSemiColon, L";")
	(TokenLeftBrace, L"{")
	(TokenRightParenthesis, L"}")
	(TokenLeftParenthesis, L"(")
	(TokenRightParenthesis, L")")
	(TokenOperatorMinus, L"-")
	(TokenId, L"identifier")
	(TokenStringLiteral, L"string literal")
	(TokenNumericLiteral, L"numeric literal")
	(TokenKeyword, L"keyword");

/// ���������ָ���token�����Ͳ���type�����׳��������
void CStatement::CheckFor(TokenType type, bool move_next)
{
	if (_iter == _tokens.end())
	{
		throw (CCompileError(*_iter, CompileErrorUnexpectedEndOfStatement, L"����������䡣"));
	}

	if ((_iter->type != type) &&
		!(type == TokenId && _iter->type == TokenKeyword && _iter->text == L"self")) // treat keyword 'self' as id
	{
		wstring message = L"Incorrect token type, " + token_map[type] + L" expected.";
		throw (CCompileError(*_iter, CompileErrorTokenType, message));
	}

	if (move_next)
	{
		++_iter;
	}
}

/// ��������ָ���token�Ƿ�ָ�������͡��ú���������������仯��
bool CStatement::IsType(TokenType type)
{
	return _iter->type != type;
}

/// ��ͼ��ȡһ��Id���������ƶ�����ȡ����֮��
std::wstring CStatement::GetId()
{
	CheckFor(TokenId, false);

	return (_iter++)->text;
}

std::wstring CStatement::GetLiteralValue()
{
	if (AtEnd() ||
		(_iter->type != TokenStringLiteral && _iter->type != TokenNumericLiteral &&
		!(_iter->type == TokenKeyword && (_iter->text == L"true" || _iter->text == L"false"))))
	{
		throw CCompileError(*_iter, CompileErrorValueExpected, L"Property value expected.");
	}

	return _iter->text;
}

void CStatement::Next()
{
	assert(_iter != _tokens.end());
	++_iter;
}

/// ��ͼ��ȡ������/��Ա�����Ի��߶˿ڣ��ԣ��������ƶ�����ȡ����֮��
std::pair<std::wstring, std::wstring> CStatement::GetProcessorMember(bool empty_member_allowed)
{
	pair<wstring, wstring> result;
	result.first = GetId();

	if (empty_member_allowed)
	{
		if (IsType(TokenOperatorDot))
		{
			Next();
			result.second = GetId();
		}
	}
	else
	{
		CheckFor(TokenOperatorDot, true);
		result.second = GetId();
	}
	return result;
}

/// ��ͼ��ȡ����id���������ƶ�����ȡ����֮��
std::wstring CStatement::GetParamId()
{
	wstring param_id;
	bool id_expected = true;
	while (_iter != _tokens.end() && _iter->type != TokenComma && _iter->type != TokenRightParenthesis)
	{
		if (!((id_expected && _iter->type == TokenId) || (!id_expected && _iter->type == TokenOperatorDot)))
		{
			throw CCompileError(*_iter, CompileErrorParamIdInvalid, L"����Id����ʽ���Ϸ���");
		}
		param_id += (_iter++)->text;
		id_expected = !id_expected;
	}

	return param_id;
}

bool Yap::CStatement::IsEmpty()
{
	return _tokens.empty();
}

void Yap::CStatement::SetType(StatementType type)
{
	_type = type;
}

Yap::StatementType Yap::CStatement::GetType() const
{
	return _type;
}

size_t Yap::CStatement::GetTokenCount() const
{
	return _tokens.size();
}

void Yap::CStatement::AddToken(const Token& token)
{
	_tokens.push_back(token);
}

const Yap::Token& Yap::CStatement::GetToken(unsigned int index)
{
	assert(index < _tokens.size()); 
	return _tokens[index];
}

const Yap::Token& Yap::CStatement::GetCurrentToken()
{
	return *_iter;
}

const Yap::Token& Yap::CStatement::GetLastToken() const
{
	return *(_tokens.end() - 1);
}

bool Yap::CStatement::AtEnd() const
{
	return _iter == _tokens.end();
}

std::wstring Yap::CStatement::GetVariableId()
{
	wstring variable_id;
	bool id_expected = true;
	while (_iter != _tokens.end() && _iter->type != TokenComma && _iter->type != TokenRightParenthesis)
	{
		if ((!id_expected && _iter->type != TokenOperatorDot) || (id_expected && _iter->type != TokenId))
		{
			throw CCompileError(*_iter, CompileErrorParamIdInvalid, L"Invalid format for system variable id.");
		}
		variable_id += (_iter++)->text;
		id_expected = !id_expected;
	}
	if (id_expected)
	{
		throw CCompileError(*(_iter - 1), CompileErrorParamIdInvalid, L"Invalid format for system variable id.");
	}

	return variable_id;
}

void Yap::CStatement::DebugOutput(wostream& output)
{
	static map<StatementType, wstring> statment_label = boost::assign::map_list_of
	(StatementImport, L"Import")
		(StatementAssign, L"Assign")
		(StatementDeclaration, L"Decl")
		(StatementPortLink, L"PortLink")
		(StatementPropertyLink, L"PropertyLink");

	output << statment_label[_type] << L"\t\t: ";

	for (auto token : _tokens)
	{
		output << token.text << " ";
	}

	output << "\n";
}

CPipelineCompiler::CPipelineCompiler(void)
{
	_key_words.insert(L"self");
	_key_words.insert(L"import");
	_key_words.insert(L"true");
	_key_words.insert(L"false");
}

CPipelineCompiler::~CPipelineCompiler(void)
{
}

CCompositeProcessor * CPipelineCompiler::Load(const wchar_t * path)
{
	wifstream script_file;
	script_file.open(path);

	if (!script_file)
	{
		wstring message = wstring(L"Failed to open script file: ") + path;
		// throw CCompileError(Token(), CompileErrorFailedOpenFile, message);
	}

	Preprocess(script_file);
	script_file.close();

	if (!_constructor)
	{
		_constructor = shared_ptr<CPipelineConstructor>(new CPipelineConstructor);
	}
	_constructor->Reset(true);

	try
	{
		if (Process())
		{
			return _constructor->GetPipeline().get();
		}

		return nullptr;
	}
	catch (CCompileError& e)
	{
		wostringstream output;
		output << L"Line " << e.GetToken().line + 1
			<< L" Column " << e.GetToken().column + 1
			<< ": Error code " << e.GetErrorNumber()
			<< " " << e.GetErrorMessage();

		wcerr << output.str();

		return nullptr;
	}
}

bool CPipelineCompiler::Preprocess(wifstream& script_file)
{
	wstring statement;
	wstring line;
	unsigned line_number = 0;

	_script_lines.clear();
	while (!_matching_check.empty()) _matching_check.pop();

	while (getline(script_file, line, L'\n'))
	{
		_script_lines.push_back(line);

		PreprocessLine(line, line_number);
		++line_number;
	}

	DebugOutputTokens(wcout);

	return true;
}

bool CPipelineCompiler::PreprocessLine(std::wstring& line, 
	int line_number)
{
	int pos = 0;

	while ((pos = int(line.find_first_not_of(L" \t\n", pos))) != wstring::npos)
	{
		switch (line[pos])
		{
		case '(':
			_tokens.push_back(Token(line_number, pos, 1, TokenLeftParenthesis));
			_matching_check.push(Token(line_number, pos, 1, TokenLeftParenthesis));
			pos += 1;
			break;
		case ')':
		{
			Token token(line_number, pos, 1, TokenRightParenthesis);
			if (_matching_check.empty())
			{
				throw CCompileError(token, CompileErrorNoMatchingLeftParenthesis, L"No matching left parethesis found.");
			}
			else if (_matching_check.top().type == TokenLeftBrace)
			{
				throw CCompileError(token, CompileErrorRightBraceExpected, L"Right brace expected.");
			}
			else
			{
				_matching_check.pop();
				pos += 1;
				_tokens.push_back(token);
			}
		}
		break;
		case '{':
			_tokens.push_back(Token(line_number, pos, 1, TokenLeftBrace));
			pos += 1;
			_matching_check.push(Token(line_number, pos, 1, TokenLeftBrace));
			break;
		case '}':
		{
			Token token(line_number, pos, 1, TokenRightBrace);
			if (_matching_check.empty())
			{
				throw CCompileError(token, CompileErrorNoMatchingLeftBrace, L"No matching left brace found.");
			}
			else if (_matching_check.top().type == TokenLeftParenthesis)
			{
				throw CCompileError(token, CompileErrorRightParenthesisExpected, L"Right parenthesis expected.");
			}
			else
			{
				_matching_check.pop();
				pos += 1;
				_tokens.push_back(token);
			}
		}
		break;
		case ';':
			_tokens.push_back(Token(line_number, pos, 1, TokenSemiColon));
			pos += 1;
			break;
		case '.':
			_tokens.push_back(Token(line_number, pos, 1, TokenOperatorDot));
			pos += 1;
			break;
		case ',':
			_tokens.push_back(Token(line_number, pos, 1, TokenComma));
			pos += 1;
			break;
		case '=':
			_tokens.push_back(Token(line_number, pos, 1, TokenOperatorAssign));
			pos += 1;
			break;
		case '\"':
		{
			int i = 1;
			for (; i + pos < int(line.length()) && line[pos + i] != '\"'; ++i);
			if (i + pos == line.length())
			{
				// ��ͬһ����û���ҵ�ƥ��Ľ�������
				throw CCompileError(Token(line_number, pos, i, TokenStringLiteral), CompileErrorNoMatchingQuote,
					L"No matching quote found on the same line. String literals must be defined on one line.");
			}

			// token�в���������
			_tokens.push_back(Token(line_number, pos + 1, i - 1, TokenStringLiteral));
			pos += i + 1;
			break;
		}
		case '/':
		{
			if (pos + 1 < int(line.length()) && line[pos + 1] == '/')
			{
				// ע�Ͳ���
				return true;
			}
			break;
		}
		case '-':
			if (pos + 1 < int(line.length()) && line[pos + 1] == '>')
			{
				_tokens.push_back(Token(line_number, pos, 2, TokenOperatorPointer));
				pos += 2;
			}
			else
			{
				_tokens.push_back(Token(line_number, pos, 1, TokenOperatorMinus));
				++pos;
			}
			break;
		case '<':
			if (pos + 2 < int(line.length()) && line[pos + 1] == '=' && line[pos + 2] == '>')
			{
				_tokens.push_back(Token(line_number, pos, 3, TokenOperatorLink));
				pos += 3;
			}
			else
			{
				throw CCompileError(Token(line_number, pos, 1, TokenUnknown), CompileErrorUnrecognizedSymbol, 
					L"Unsupported symbol:\'<\'");
			}
			break;
		default:
		{
			size_t next_separator = -1;
			Token token;
			token.line = line_number;
			token.column = pos;

			if (isdigit(line[pos]))
			{
				token.type = TokenNumericLiteral;
				next_separator = line.find_first_of(L" \t\n\"{}()+-,*/=<>;", pos);
			}
			else if (isalpha(line[pos]))
			{
				token.type = TokenId;
				next_separator = line.find_first_of(L" \t\n\"{}()+-.,*/=<>;", pos);
			}
			else
			{
				throw CCompileError(Token(line_number, pos, 1, TokenUnknown), CompileErrorUnrecognizedSymbol, 
					L"Unrecognized symbol.");
			}

			token.length = int(((next_separator == -1) ? line.length() : next_separator) - token.column);

			if (token.type == TokenId)
			{
				auto token_string = line.substr(token.column, token.length);
				if (_key_words.find(token_string) != _key_words.end())
				{
					token.type = TokenKeyword;
				}
			}

			_tokens.push_back(token);
			pos = int(next_separator);

			if (next_separator == -1)
			{
				break;
			}
		}
		}
	}

	return true;
}

bool CPipelineCompiler::Process()
{
	CStatement statement;
	statement.Reset();

	for (auto token : _tokens)
	{
		token.text = GetTokenString(token);

		switch (token.type)
		{
		case TokenKeyword:
			if (token.text == L"import" && statement.IsEmpty())
			{
				statement.SetType(StatementImport);
			}
			break;
		case TokenOperatorAssign:
			if (statement.GetType() == StatementUnknown)
			{
				statement.SetType(StatementAssign);
			}
			break;
		case TokenOperatorLink:
			if (statement.GetType() == StatementUnknown)
			{
				statement.SetType(StatementPropertyLink);
			}
			break;
		case TokenOperatorPointer:
			statement.SetType(StatementPortLink);
			break;
		case TokenId:
			if (statement.GetTokenCount() == 1 && statement.GetToken(0).type == TokenId)
			{
				statement.SetType(StatementDeclaration);
			}
			break;
		case TokenSemiColon:
			if (!statement.IsEmpty())
			{
				if (statement.GetType() == StatementUnknown)
				{
					throw(CCompileError(statement.GetLastToken(), 
						CompileErrorIncompleteStatement, L"Statement not complete."));
				}
#ifdef DEBUG
				DebugOutputStatement(cout, statement);
#endif
				ProcessStatement(statement);
				statement.Reset();
			}
			break;
		default:
			;
		}

		if (token.type != TokenSemiColon)
		{
			statement.AddToken(token);
		}
	}

	return true;
}


bool CPipelineCompiler::ProcessStatement(CStatement& statement)
{
	switch (statement.GetType())
	{
	case StatementImport:
		return ProcessImport(statement);
	case StatementAssign:
		return ProcessAssignment(statement);
	case StatementDeclaration:
		return ProcessDeclaration(statement);
	case StatementPortLink:
		return ProcessPortLink(statement);
	case StatementPropertyLink:
		return ProcessPropertyLink(statement);
	default:
		assert(0);
		return false;
	}
}

wstring CPipelineCompiler::GetTokenString(const Token& token) const
{
	return _script_lines[token.line].substr(token.column, token.length);
}

/**
	������ָ��ģ�����ơ�
*/
bool CPipelineCompiler::ProcessImport(CStatement& statement)
{
	assert(statement.GetType() == StatementImport);
	assert(!statement.IsEmpty());

	if (statement.GetTokenCount() > 2)
	{
		throw CCompileError(statement.GetToken(2), CompileErrorSemicolonExpected, L"Semicolon expected.");
	}

	statement.Begin();
	statement.Next(); // bypass 'import'

	const Token& token = statement.GetCurrentToken();
	if (statement.GetTokenCount() == 1 || token.type != TokenStringLiteral)
	{
		throw CCompileError(token, CompileErrorStringExpected, 
			L"String literal should be used to specify plugin to import.");
	}

	if (!_constructor->LoadModule(token.text.c_str()))
	{
		wstring output = wstring(L"�޷�����ģ���ļ���") + token.text;
		throw CCompileError(token, CompileErrorLoadModule, output);
	}

	return true;
}

/**
������ָ���һ��Ԫ�ء�һ����ʽ�ǣ�
process1.output_port->process2.input_port;
����.output_port��.input_port�ǿ�ѡ�ģ����ʡ�ԣ���ʹ��ȱʡ��Output��Input��
*/
bool CPipelineCompiler::ProcessPortLink(CStatement& statement)
{
	assert(_constructor);

	statement.Begin();
	auto source_processor = statement.GetId();

	wstring source_port(L"Output");
	if (statement.GetCurrentToken().type == TokenOperatorDot)
	{
		statement.Next();
		source_port = statement.GetId();
	}

	statement.CheckFor(TokenOperatorPointer, true);

	auto dest_processor = statement.GetId();

	wstring dest_port(L"Input");

	if (statement.GetCurrentToken().type == TokenOperatorDot)
	{
		statement.Next();
		dest_port = statement.GetId();
	}

	if (source_processor == L"self")
	{
		if (dest_processor == L"self")
		{
			throw CCompileError(statement.GetCurrentToken(), CompileErrorSelfLink, 
				L"Can not link the output of the pipeline to the input of itself.");
		}
		else
		{
			return _constructor->AssignPipelineInPort(source_port.c_str(), dest_processor.c_str(), dest_port.c_str());
		}
	}
	else if (dest_processor == L"self")
	{
		return _constructor->AssignPipelineOutPort(dest_port.c_str(), source_processor.c_str(), source_port.c_str());
	}
	else
	{
		return _constructor->Link(source_processor.c_str(), source_port.empty() ? L"Output" : source_port.c_str(),
			dest_processor.c_str(), dest_port.empty() ? L"Input" : dest_port.c_str());
	}
}


bool CPipelineCompiler::ProcessDeclaration(CStatement& statement)
{
	assert(statement.GetType() == StatementDeclaration);
	assert(statement.GetTokenCount() >= 2);

	statement.Begin();

	wstring class_id = statement.GetId();
	wstring instance_id = statement.GetId();

	if (_constructor->InstanceIdExists(instance_id.c_str()))
	{
		throw CCompileError(statement.GetToken(1), CompileErrorIdExists, 
			wstring(L"Instance id specified for the processor already exists: ") + instance_id);
	}

	if (statement.AtEnd())
	{
		_constructor->CreateProcessor(class_id.c_str(), instance_id.c_str());
	}
	else if (statement.GetCurrentToken().type != TokenLeftParenthesis)
	{
		throw CCompileError(statement.GetCurrentToken(), CompileErrorSemicolonExpected, 
			L"Semicolon or left parenthesis expected.");
	}
	else
	{
		_constructor->CreateProcessor(class_id.c_str(), instance_id.c_str());
		for (;;)
		{
			statement.Next();
			wstring property = statement.GetId();

			if (statement.GetCurrentToken().type == TokenOperatorAssign)
			{
				statement.Next();
				wstring value = statement.GetLiteralValue();
				_constructor->SetProperty(instance_id.c_str(), property.c_str(), value.c_str());
			}
			else if (statement.GetCurrentToken().type == TokenOperatorLink)
			{
				statement.Next();
				wstring variable_id = statement.GetVariableId();

				_constructor->LinkProperty(instance_id.c_str(), property.c_str(), variable_id.c_str());
			}
			else
			{
				throw CCompileError(statement.GetCurrentToken(), CompileErrorPropertyOperatorExpected, 
					L"Property operator must be specified, you can use either \'=\' or \'<=>\'.");
			}

			if (statement.AtEnd())
			{
				throw CCompileError(statement.GetLastToken(), CompileErrorRightParenthesisExpected, L"Right parenthesis expected.");
			}
			else if (statement.GetCurrentToken().type == TokenRightParenthesis)
			{
				break;
			}
			else if (statement.GetCurrentToken().type != TokenComma)
			{
				throw CCompileError(statement.GetCurrentToken(), CompileErrorCommaExpected, 
					L"Comma \',\' or right parenthesis \')\' expected.");
			}
		}
	}

	return true;
}

bool CPipelineCompiler::ProcessPropertyLink(CStatement& statement)
{
	assert(statement.GetType() == StatementPropertyLink);

	statement.Begin();

	wstring processor_instance_id = statement.GetId();
	if (!_constructor->InstanceIdExists(processor_instance_id.c_str()))
	{
		throw CCompileError(statement.GetToken(0), CompileErrorProcessorNotFound, 
			wstring(L"Processor not found: ") + processor_instance_id);
	}

	statement.CheckFor(TokenOperatorDot, true);
	wstring property = statement.GetId();

	statement.CheckFor(TokenOperatorLink, true);
	wstring variable_id = statement.GetVariableId();

	return _constructor->LinkProperty(processor_instance_id.c_str(), property.c_str(), variable_id.c_str());
}

bool CPipelineCompiler::ProcessAssignment(CStatement& statement)
{
	assert(statement.GetType() == StatementAssign);

	statement.Begin();

	wstring processor_instance_id = statement.GetId();
	if (!_constructor->InstanceIdExists(processor_instance_id.c_str()))
	{
		throw CCompileError(statement.GetToken(0), CompileErrorProcessorNotFound,
			wstring(L"Processor not found: ") + processor_instance_id);
	}

	statement.CheckFor(TokenOperatorDot, true);
	wstring property = statement.GetId();

	statement.CheckFor(TokenOperatorAssign, true);
	wstring value = statement.GetLiteralValue();

	return _constructor->SetProperty(processor_instance_id.c_str(), property.c_str(), value.c_str());	
}

void CPipelineCompiler::DebugOutputTokens(std::wostream& output)
{
	for (auto iter = _tokens.begin(); iter != _tokens.end(); ++iter)
	{
		auto token = _script_lines[iter->line].substr(iter->column, iter->length);
		auto token_item = token_map.find(iter->type);
		if (token_item != token_map.end())
		{
			output << token_item->second.c_str() << "\t: " << token.c_str() << "\n";
		}
		else
		{
			assert(0);
		}
	}
}


void CPipelineCompiler::TestTokens()
{
	map<TokenType, wstring> token_map = boost::assign::map_list_of(TokenOperatorPointer, L"->")
		(TokenOperatorLink, L"<=>")
		(TokenOperatorDot, L".")
		(TokenOperatorAssign, L"=")
		(TokenComma, L",")
		(TokenSemiColon, L";")
		(TokenLeftBrace, L"{")
		(TokenRightParenthesis, L"}")
		(TokenLeftParenthesis, L"(")
		(TokenRightParenthesis, L")")
		(TokenOperatorMinus, L"-");

	for (auto iter = _tokens.begin(); iter != _tokens.end(); ++iter)
	{
		auto token = _script_lines[iter->line].substr(iter->column, iter->length);
		auto token_item = token_map.find(iter->type);
		if (token_item != token_map.end())
		{
			assert(token_item->second == token);
		}
		else
		{
			switch (iter->type)
			{
			case TokenId:
				assert(isalpha(token[0]));
				break;
			case TokenStringLiteral:
				assert(_script_lines[iter->line][iter->column - 1] == L'\"' &&
					_script_lines[iter->line][iter->column + iter->length] == L'\"');
				break;
			case TokenNumericLiteral:
				assert(isdigit(token[0]));
				break;
			case TokenKeyword:
				assert(_key_words.find(token) != _key_words.end());
				break;
			default:
				assert(0);
			}
		}
	}
}
