/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jerperez <jerperez@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 15:21:14 by jerperez          #+#    #+#             */
/*   Updated: 2024/09/04 16:29:42 by jerperez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tokenizer.hpp"
#include <string> 
#include <cstring>

const char	*TokenizerException::what(void) const throw()
{
	return ("TMP");
}

// static void	_debug_show_token(token_deq_t &list)
// {
// 	int			token_id;

// 	token_deq_t::iterator	it_end = list.end();
// 	for (token_deq_t::iterator it = list.begin(); it != it_end; ++it)
// 	{
// 		token_id = it->token_id;
// 		if (-1 == token_id)
// 			std::cout << it->word;
// 		else
// 		{
// 			if ('\n' != token_id && isspace(token_id))
// 				std::cout << "\e[33m_\e[0m";
// 			else
// 				std::cout << "\e[33m" << (char)(token_id) << "\e[0m";
// 		}
// 	}
// }

static void	_add_c_to_token(unsigned char c, int quote_char, std::string &word, token_deq_t &list)
{
	char	*p_c;

	if (0 == quote_char)
	{
		p_c = (char *)(std::strchr(TK_METACHAR, c));
		if (NULL != p_c)
		{
			t_token	tk;
			if (false == word.empty())
			{
				tk.word = word;
				//std::cout << "word:" << word << std::endl;
				tk.token_id = -1;
				list.push_back(tk);
				word.erase();
			}
			tk.word = "";
			tk.token_id = *p_c;
			list.push_back(tk);
		}
		else
			word += c;
	}
	else
		word += c;
}

static void	_add_token_quote(std::string line, token_deq_t &list)
{
	const int		length = line.length();
	int				quote_char;
	unsigned char	c;
	std::string		word("");

	quote_char = 0;
	for (int i = 0; i < length; ++i)
	{
		c = line[i];
		if (0 == quote_char)
		{
			if (TK_QUOTE == c)
				quote_char = TK_QUOTE;
			else if (TK_ESCAPE == c)
				quote_char = TK_ESCAPE;
			_add_c_to_token(c, quote_char, word, list);
		}
		else if (TK_QUOTE == quote_char)
		{
			if (TK_QUOTE == c)
				quote_char = 0;
			_add_c_to_token(c, quote_char, word, list);
		}
		else if (TK_ESCAPE == quote_char)
		{
			_add_c_to_token(c, TK_ESCAPE, word, list);
			quote_char = 0;
		}
		else
			_add_c_to_token(c, quote_char, word, list);
	}
	if (0 != quote_char)
		throw TokenizerException(); //
	_add_c_to_token('\n', 0, word, list);
	if (false == word.empty())
		throw TokenizerException(); //
}

token_deq_t	tk_tokenize(std::fstream &input_file)
{
	std::string			line;
	token_deq_t			list;

	while (std::getline(input_file, line))
		_add_token_quote(line, list);
	//_debug_show_token(list); //
	return (list);
}
