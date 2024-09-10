/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jerperez <jerperez@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 15:20:58 by jerperez          #+#    #+#             */
/*   Updated: 2024/09/10 10:48:50 by jerperez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include "tokenizer.hpp"
#include "parser.hpp"
#include "Directive.hpp"
#include "BlockDirective.hpp"
#include "InvalidDirective.hpp"
#include <iomanip>
#include <cstdlib>

static void	_print_tokens(token_deq_t list)
{
	token_deq_t::const_iterator	it_end = list.end();

	for (token_deq_t::iterator	it = list.begin(); it != it_end; ++it)
	{
		if (-1 == it->token_id)
			std::cout << it->word;
		else if (' ' == it->token_id || '\t' == it->token_id)
			std::cout << "\e[33m_\e[0m";
		else if ('\n' == it->token_id)
			std::cout << "\e[33m(newline)\n\e[0m";
		else
			std::cout << "\e[33m" << (char)(it->token_id) << "\e[0m";
	}
	std::cout << std::endl;
}

/* example function
 * args for arguments, context for calling directive (ex: main, server, etc, ../)
 * PLACEHOLDER add virtual server object to change properties
 */
static int	_listen(Directive::funmap_arg_t args, Directive *context)
{
	Directive::funmap_arg_t::iterator	it_end = args.end();
	Directive::funmap_arg_t::iterator	it = args.begin();

	(void)context;
	if (it == it_end)
		return (1);
	++it;
	if (it == it_end || it->empty())
		return (2);
	
	const int	port = std::atoi(it->c_str());

	if (0 > port)
		return (2);
	++it;
	if (it != it_end)
		return (2);
	std::cout << "PLACEHOLDER listening to " << port << std::endl; // Do stuff here
	return (0);
}

static void	_print_dir(Directive *d)
{
	int								type = d->getType();
	std::deque<std::string>			args = d->getArgs();
	if (PR_DIR_TYPE_BLOCK == type)
		std::cout << "BLOCK:\t\t[" << std::setw(10) <<  d->getContext() << std::setw(10) << (reinterpret_cast<BlockDirective*>(d))->getContext()->getArgs().front() << "]\tArgs:\t";
	else if (PR_DIR_TYPE_SIMPLE == type)
		std::cout << "DIRECTIVE:\t[" <<  std::setw(10) <<  d->getContext() << std::setw(10) << (reinterpret_cast<BlockDirective*>(d))->getContext()->getArgs().front() << "]\tArgs:\t";
	else if (-1 == type)
		std::cout <<  std::setw(10) <<  (reinterpret_cast<InvalidDirective*>(d))->getWhat() <<  "[" << std::setw(10) <<  d->getContext() << std::setw(10) << (reinterpret_cast<BlockDirective*>(d))->getContext()->getArgs().front() << "]\tArgs:\t";
	for (std::deque<std::string>::iterator it = args.begin(); it != args.end(); ++it)
		std::cout << *it << "\t";
	std::cout << std::endl;
	if (PR_DIR_TYPE_BLOCK == type)
	{
		std::deque<Directive*>			instruction = (reinterpret_cast<BlockDirective*>(d))->getInstructions();
		for (std::deque<Directive*>::iterator it = instruction.begin(); it != instruction.end(); ++it)
			_print_dir(*it);
	}
}

static int	_read(char *pathname)
{
	std::fstream 					f;
	token_deq_t						list;
	Directive						*d;
	Directive						*context;
	Directive::funmap_t				evalmap;

	evalmap["listen"] = &_listen;
	f.open(pathname);
	if (false == f.is_open())
	{
		std::cerr << "error: fstream: Failed to open configuration file" << std::endl;
		return (1);
	}
	context = new BlockDirective();
	if (tk_tokenize(f, list))
	{
		std::cerr << "error: tokenizer: Failed to tokentize" << std::endl;
		return (1);
	}
	_print_tokens(list);
	std::cout << "\e[34m##############################\e[0m" << std::endl;
	token_deq_t::iterator	it_curr = list.begin();
	d = pr_next_directive(it_curr, list.end(), context);
	if (d && -1 == d->getType())
	{
		_print_dir(d);
		delete d;
		d = 0;
	}
	while (0 != d)
	{
		_print_dir(d);
		if (0 != d->evaluate(evalmap))
			std::cerr << "\e[33mfailed to evaluate" <<  std::setw(10) << d << "\e[0m" << std::endl;
		else
			std::cerr << "\e[32msuccessful\e[0m" << std::endl;
		delete d;
		d = pr_next_directive(it_curr, list.end(), context);
		if (d && -1 == d->getType())
		{
			_print_dir(d);
			delete d;
			d = 0;
		}
	}
	delete context;
	f.close();
	return (0);
}


int	main(int ac, char *av[])
{
	if (2 == ac)
		return (_read(av[1]));
	return (2);
}
