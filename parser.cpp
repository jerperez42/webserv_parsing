/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jerperez <jerperez@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 17:54:04 by jerperez          #+#    #+#             */
/*   Updated: 2024/09/06 15:21:58 by jerperez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "InvalidDirective.hpp"
#include "BlockDirective.hpp"
#include "Directive.hpp"
#include "tokenizer.hpp"
#include "parser.hpp"
#include <string>
#include <cstring>

const char* ParserUnclosedBlockException::what(void) const throw()
{
	return ("WIP");
}

const char* ParserUnclosedDirectionException::what(void) const throw()
{
	return ("WIP");
}

static bool	_isspace(char c)
{
	return (NULL != (char *)(std::strchr(TK_SPACE, c)));
}

static bool	_next_simple_directive(
	token_deq_t::iterator &it_list,
	token_deq_t::const_iterator it_end,
	std::deque<std::string> &args)
{
	while (it_list != it_end && _isspace(it_list->token_id))
		++it_list;
	while (it_list != it_end && '{' != it_list->token_id)
	{
		//std::cerr << "D:\t" << it_list->token_id << "\t" << it_list->word << std::endl;
		if (-1 == it_list->token_id)
			args.push_back(it_list->word);
		else if (';' == it_list->token_id)
			return (true);
		else if ('}' == it_list->token_id)
			return (false);
		if (it_list != it_end)
			++it_list;
		while (it_list != it_end && _isspace(it_list->token_id))
			++it_list;
	}
	return (false);
}

static bool	_next_block_directive(
	token_deq_t::iterator &it_list,
	token_deq_t::const_iterator it_end,
	BlockDirective *curr)
{
	Directive	*d;

	while (it_list != it_end)
	{
		//std::cerr << "B:\t" << it_list->token_id << "\t" << it_list->word << std::endl;
		if ('{' == it_list->token_id)
			return (false);
		else if ('}' == it_list->token_id)
			return (true);
		else
		{
			d = pr_next_directive(it_list, it_end, curr);
			if (d && -1 == d->getType())
				return (false);
			curr->addInstruction(d);
		}
		while (it_list != it_end && _isspace(it_list->token_id))
			++it_list;
	}
	return (false);
}

Directive	*pr_next_directive(
	token_deq_t::iterator &it_list,
	token_deq_t::const_iterator it_end,
	Directive *context)
{
	std::deque<std::string>	args;

	if (0 == context)
		return (new InvalidDirective(args, context, "null context"));
	if (true == _next_simple_directive(it_list, it_end, args))
	{
		++it_list;
		return (new Directive(args, context));
	}
	if (true == args.empty())
	{
		if (it_list == it_end)
			return (0);
		else if (';' == it_list->token_id)
			return (0);
		else if ('}' == it_list->token_id)
		{
			std::cerr << "context:" << context << std::endl;
			if (true == reinterpret_cast<BlockDirective*>(context)->closeBlock())
				return (0);
			else
				return (new InvalidDirective(args, context, "unexpected } token, no arguments"));
		}
	}
	else if ((it_list == it_end) || ('}' == it_list->token_id))
		return (new InvalidDirective(args, context, "bad directive"));
	else if ('{' == it_list->token_id)
		++it_list;
	else if (false == args.empty())
		return (new InvalidDirective(args, context, "missing expected ; token"));

	BlockDirective			*new_block;

	if (true == args.empty())
		return (new InvalidDirective(args, context, "block directive missing name"));
	new_block = new BlockDirective(args, context);
	new_block->openBlock();
	if (true == _next_block_directive(it_list, it_end, new_block))
	{
		++it_list; //
		return (new_block);
	}
	if (false == new_block->getInstructions().empty())
	{
		delete new_block;
		return (new InvalidDirective(args, context, "missing expected } token"));
	}
	delete new_block;
	return (new InvalidDirective(args, context, "missing expected } token, no instructions"));
}
