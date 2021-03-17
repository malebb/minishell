/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_cmd.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbrunet <clbrunet@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/28 07:16:13 by clbrunet          #+#    #+#             */
/*   Updated: 2021/03/17 15:02:13 by mlebrun          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parsing.h"
#include "ft.h"
#include <stdio.h>

void	skip_quote(char const *str, int *i)
{
	int		to_escape;

	to_escape = 0;
	*i = *i + 1;
	while (!(str[*i] == '"' && !to_escape))
	{
		if (str[*i] == '\\' && !to_escape)
			to_escape = 1;
		else
			to_escape = 0;
		*i = *i + 1;
	}
	*i = *i + 1;
}

void	skip_single_quote(char const *str, int *i)
{
	*i = *i + 1;
	while (str[*i] != '\'')
		*i = *i + 1;
	*i = *i + 1;
}

void	skip_no_quote(char const *str, int *i, int len)
{
	int		to_escape;

	to_escape = 0;
	while (str[*i] != ' ' && !(str[*i] == '|' && !to_escape) && *i < len)
	{
		if (str[*i] == '\\' && !to_escape)
			to_escape = 1;
		else
			to_escape = 0;
		*i = *i + 1;
	}
}

static void	skip_redirection(char const *str, int *i, int len)
{
	*i = *i + 1;
	if ((str[*i] == '>' || str[*i] == '<') && *i < len)
		*i = *i + 1;
	while (1)
	{
		while (str[*i] == ' ')
			*i = *i + 1;
		if (str[*i] == '"')
			skip_quote(str, i);
		else if (str[*i] == '\'')
			skip_single_quote(str, i);
		else
			skip_no_quote(str, i, len);
		while (str[*i] == ' ')
			*i = *i + 1;
		if ((str[*i] == '<' || str[*i] == '>') && *i < len)
		{
			*i = *i + 1;
			if ((str[*i] == '>' || str[*i] == '<') && *i < len)
				*i = *i + 1;
		}
		else
			break ;
	}
}

static int	count_arg(char const *str, int i, int len)
{
	int		count;
	int		to_escape;

	to_escape = 0;
	count = 0;
	while (i < len)
	{
		while (str[i] == ' ')
			i++;
		if ((str[i] == '>' || str[i] == '<'))
			skip_redirection(str, &i, len);
		if (str[i] == '|' && !to_escape)
			return (count);
		if (i != len)
			count++;
		if (str[i] == '"' && !to_escape)
			skip_quote(str, &i);
		else if (str[i] == '\'' && !to_escape)
			skip_single_quote(str, &i);
		else
			skip_no_quote(str, &i, len);
	}
	return (count);
}

int	size_var(t_parse_cmd p, int i, int j, int size_name)
{
	int		o;
	int		size_value;

	if (ft_strncmp(p.envp[j], p.str_cmd + i, size_name) == 0)
	{
		if (p.envp[j][size_name] == '=')
		{
			o = 1;
			size_value = 0;
			while (p.envp[j][size_name + o] != '\0')
			{
				size_value++;
				o++;
			}
			return (size_value);
		}
	}
	return (-1);
}

void	cpy_var_value(t_parse_cmd *p, int size_name, int k, int *j)
{
	int		size_var;
	int		o;

	size_var = 0;
	while (p->envp[k][size_name + 1 + size_var] != '\0')
		size_var++;
	o = 0;
	while (p->envp[k][size_name + o + 1] != '\0')
	{
		p->buf[o + *j] = p->envp[k][size_name + o + 1];
		o++;
	}
	p->buf[o + *j] = '\0';
	*j = *j + size_var;
}

int		cpy_var(t_parse_cmd *p, int i, int *j, int size_name)
{
	int		k;

	k = 0;
	while (p->envp[k] != NULL)
	{
		if (ft_strncmp(p->envp[k], p->str_cmd + i + 1, size_name) == 0)
		{
			if (p->envp[k][size_name] == '=')
				cpy_var_value(p, size_name, k, j);
			return (1);
		}
		k++;
	}
	return (-1);
}

int	fill_dollar(t_parse_cmd *p, int i, int *j, int to_escape)
{
	int		size_name;
	int		error;

	size_name = 0;
	if (p->str_cmd[i] == '$' && !to_escape && !ft_isalpha(p->str_cmd[i + 1])
		&& p->str_cmd[i] != '_')
	{
		p->buf[*j] = '$';
		p->buf[*j + 1] = '\0';
		*j = *j + 1;
	}
	else if (p->str_cmd[i] == '$' && !to_escape)
	{
		while (ft_isalnum(p->str_cmd[(i + 1) + size_name]))
			size_name++;
		error = cpy_var(p, i, j, size_name);
		if (error != -1)
			return (size_name);
	}
	return (size_name);
}

int	is_meta_char(char c)
{
	if (c == '>' || c == '<' || c == '*' || c == '?' || c == '['
		|| c == ']' || c == '|' || c == ';' || c == '&' || c == '('
		|| c == ')' || c == '#' || c == '$' || c == '\\' || c == '{'
		|| c == '}' || c == '"' || c == '\'' || c == '`' || c == ';'
		|| c == '=')
		return (1);
	return (0);
}

int	is_meta_char_quote(char c)
{
	if (c == '$' || c == '\\' || c == '"' || c == '`')
		return (1);
	return (0);
}

void	add_char_quote(t_parse_cmd *p, int *i, int *j, int to_escape)
{
	if (!is_meta_char_quote(p->str_cmd[*i]) && to_escape)
	{
		p->buf[*j] = '\\';
		*j = *j + 1;
		p->buf[*j] = p->str_cmd[*i];
		*j = *j + 1;
	}
	else if (!is_meta_char_quote(p->str_cmd[*i]) ||
			(is_meta_char_quote(p->str_cmd[*i]) && to_escape))
	{
		p->buf[*j] = p->str_cmd[*i];
		*j = *j + 1;
	}
}

void	fill_quote(t_parse_cmd *p, int *i, int *j)
{
	int		to_escape;
	int		name_size;

	to_escape = 0;
	*i = *i + 1;
	while (!(p->str_cmd[*i] == '"' && !to_escape))
	{
		if (p->str_cmd[*i] == '\\' && !to_escape)
			to_escape = 1;
		else
		{
			add_char_quote(p, i, j, to_escape);
			name_size = fill_dollar(p, *i, j, to_escape);
			*i = *i + name_size;
			to_escape = 0;
		}
		*i = *i + 1;
	}
	*i = *i + 1;
}

void	fill_no_quote(t_parse_cmd *p, int *i, int *j, int len)
{
	int		to_escape;
	int		name_size;

	to_escape = 0;
	while (*i < len && p->str_cmd[*i] != ' ' &&
	!(p->str_cmd[*i] == '<' && !to_escape) && !(p->str_cmd[*i] == '>' &&
	!to_escape) && !(p->str_cmd[*i] == '|' && !to_escape))
	{
		if (p->str_cmd[*i] == '\\' && !to_escape)
			to_escape = 1;
		else
		{
			if (!is_meta_char(p->str_cmd[*i]) ||
			(is_meta_char(p->str_cmd[*i]) && to_escape))
			{
				p->buf[*j] = p->str_cmd[*i];
				*j = *j + 1;
			}
			name_size = fill_dollar(p, *i, j, to_escape);
			*i = *i + name_size;
			to_escape = 0;
		}
		*i = *i + 1;
	}
}

static void	fill_buf(t_parse_cmd *p, int len, int i)
{
	int		to_escape;
	int		j;

	j = 0;
	to_escape = 0;
	if (p->str_cmd[i] == '"')
		fill_quote(p, &i, &j);
	else if (p->str_cmd[i] == '\'')
	{
		i++;
		while (p->str_cmd[i] != '\'')
		{
			p->buf[j] = p->str_cmd[i];
			j++;
			i++;
		}
	}
	else
		fill_no_quote(p, &i, &j, len);
	p->buf[j] = '\0';
}

static char	**init_args(int	arg_nb, t_parse_cmd p)
{
	char		**args;
	int			i;

	args = malloc(sizeof(char *) * (arg_nb + 2));
	if (!args)
		return (0);
	i = 0;
	while (i < (arg_nb + 2))
	{
		args[i] = NULL;
		i++;
	}
	args[0] = ft_strdup(p.cmd->exe);
	if (!args[0])
	{
		free(args);
		return (0);
	}
	args[arg_nb + 1] = NULL;
	return (args);
}

void	real_quote_size(t_parse_cmd p, int *size, int *i, int len)
{
	int		to_escape;

	*size = *size + 1;
	*i = *i + 1;
	to_escape = 0;
	while (*i < len && !(p.str_cmd[*i] == '"' && !to_escape))
	{
		if (p.str_cmd[*i] == '\\' && !to_escape)
			to_escape = 1;
		else
			to_escape = 0;
		*size = *size + 1;
		*i = *i + 1;
	}
	*size = *size + 1;
}

void	real_single_quote_size(t_parse_cmd p, int *i, int *size)
{
	*size = *size + 1;
	*i = *i + 1;
	while (p.str_cmd[*i] != '\'')
	{
		*size = *size + 1;
		*i = *i + 1;
	}
	*size = *size + 1;
}

int	real_component_size(t_parse_cmd p, int i, int len)
{
	int		size;
	int		to_escape;

	to_escape = 0;
	size = 0;
	if (p.str_cmd[i] == '"')
		real_quote_size(p, &size, &i, len);
	else if (p.str_cmd[i] == '\'')
		real_single_quote_size(p, &i, &size);
	else
	{
		while (i < len && p.str_cmd[i] != ' ' && !(p.str_cmd[i] == '|' &&
			!to_escape) && !(p.str_cmd[i] == '>' && !to_escape)
			&& !(p.str_cmd[i] == '<' && !to_escape))
		{
			if (p.str_cmd[i] == '\\' && !to_escape)
				to_escape = 1;
			else
				to_escape = 0;
			size++;
			i++;
		}
	}
	return (size);
}

int		search_var(t_parse_cmd p, int *i, int size_name, int *size)
{
	int		j;
	int		size_value;

	j = 0;
	while (p.envp[j] != NULL)
	{
		size_value = size_var(p, (*i + 1), j, size_name);
		if (size_value != -1)
		{
			*size = *size + size_value;
			return (size_name);
		}
		j++;
	}
	return (size_name);
}

int	dollar_size(t_parse_cmd p, int *size, int *i, int to_escape)
{
	int		size_name;

	size_name = 0;
	if (p.str_cmd[*i] == '$' && !to_escape && !ft_isalpha(p.str_cmd[*i + 1])
		&& p.str_cmd[*i] != '_')
	{
		*size = *size + 1;
		return (0);
	}
	if (p.str_cmd[*i] == '$' && !to_escape)
	{
		while (ft_isalnum(p.str_cmd[(*i + 1) + size_name]))
			size_name++;
		size_name = search_var(p, i, size_name, size);
		return (size_name);
	}
	return (0);
}

void	size_quote(t_parse_cmd p, int *i, int *size)
{
	int		to_escape;
	int		name_size;

	to_escape = 0;
	*i = *i + 1;
	while (!(p.str_cmd[*i] == '"' && !to_escape))
	{
		if (p.str_cmd[*i] == '\\' && !to_escape)
		{
			to_escape = 1;
			*size = *size + 1;
		}
		else
		{
			if (!is_meta_char_quote(p.str_cmd[*i]))
				*size = *size + 1;
			name_size = dollar_size(p, size, i, to_escape);
			*i = *i + name_size;
			to_escape = 0;
		}
		*i = *i + 1;
	}
	*i = *i + 1;
}

void	size_no_quote(t_parse_cmd p, int *i, int *size, int len)
{
	int		to_escape;
	int		name_size;

	to_escape = 0;
	while (*i < len && p.str_cmd[*i] != ' ' && 
			!(p.str_cmd[*i] == '>' && !to_escape) &&
			!(p.str_cmd[*i] == '<' && !to_escape) &&
			!(p.str_cmd[*i] == '|' && !to_escape))
	{
		if (p.str_cmd[*i] == '\\' && !to_escape)
		{
			to_escape = 1;
			*size = *size + 1;
		}
		else
		{
			if (!is_meta_char(p.str_cmd[*i]))
				*size = *size + 1;
			name_size = dollar_size(p, size, i, to_escape);
			*i = *i + name_size;
			to_escape = 0;
		}
		*i = *i + 1;
	}
}

int	size_component_formated(t_parse_cmd p, int i, int len)
{
	int		size;

	size = 0;
	if (p.str_cmd[i] == '"')
		size_quote(p, &i, &size);
	if (p.str_cmd[i] == '\'')
	{
		i++;
		while (p.str_cmd[i] != '\'')
		{
			i++;
			size++;
		}
		i++;
	}
	else
		size_no_quote(p, &i, &size, len);
	return (size);
}

static t_redirection	*create_red(char *path_or_endstr)
{
	t_redirection		*red;

	red = malloc(sizeof(t_redirection) * (1));
	if (!red)
		return (NULL);
	red->next = NULL;
	red->path_or_endstr = path_or_endstr;
	red->next = NULL;
	red->type = NONE;
	return (red);
}

void	add_out_red(t_parse_cmd *p, t_redirection_type red_type,
					char *path_or_endstr)
{
	t_redirection	*first_red;
	t_redirection	*red;

	red = create_red(path_or_endstr);
	first_red = p->cmd->out_redirection;
	while (p->cmd->out_redirection && p->cmd->out_redirection->next != NULL)
		p->cmd->out_redirection = p->cmd->out_redirection->next;
	if (red_type == 1)
		red->type = SIMPLE;
	else
		red->type = DOUBLE;
	if (p->cmd->out_redirection)
		p->cmd->out_redirection->next = red;
	else
		p->cmd->out_redirection = red;
	if (first_red != NULL)
		p->cmd->out_redirection = first_red;
}

void	add_in_red(t_parse_cmd *p, t_redirection_type red_type,
					char *path_or_endstr)
{
	t_redirection	*first_red;
	t_redirection	*red;

	red = create_red(path_or_endstr);
	first_red = p->cmd->in_redirection;
	while (p->cmd->in_redirection && p->cmd->in_redirection->next != NULL)
		p->cmd->in_redirection = p->cmd->in_redirection->next;
	if (red_type == 1)
		red->type = SIMPLE;
	else
		red->type = DOUBLE;
	if (p->cmd->in_redirection)
		p->cmd->in_redirection->next = red;
	else
		p->cmd->in_redirection = red;
	if (first_red != NULL)
		p->cmd->in_redirection = first_red;
}

static void	add_red(t_parse_cmd *p, char *path_or_endstr, int in_out,
					t_redirection_type red_type)
{
	if (in_out)
		add_out_red(p, red_type, path_or_endstr);
	else
		add_in_red(p, red_type, path_or_endstr);
}

void	determine_in_type(t_parse_cmd *p, int *in_out, int *i,
		t_redirection_type *red_type)
{
	*in_out = 1;
	*i = *i + 1;
	if (p->str_cmd[*i] == '>')
	{
		*i = *i + 1;
		*red_type = DOUBLE;
	}
	else
		*red_type = SIMPLE;
}

int		determine_red_type(t_parse_cmd *p, int *i,
		t_redirection_type *red_type, int *in_out)
{
	if (p->str_cmd[*i] == '>')
		determine_in_type(p, in_out, i, red_type);
	else if (p->str_cmd[*i] == '<')
	{
		*in_out = 0;
		*i = *i + 1;
		if (p->str_cmd[*i] == '<')
		{
			*i = *i + 1;
			*red_type = DOUBLE;
		}
		else
			*red_type = SIMPLE;
	}
	else
		return (0);
	return (1);
}

void	skip_char_red(t_parse_cmd *p, int *i, int len)
{
	int		to_escape;

	to_escape = 0;
	if (p->str_cmd[*i] == '"')
		skip_quote(p->str_cmd, i);
	else if (p->str_cmd[*i] == '\'')
		skip_single_quote(p->str_cmd, i);
	else
	{
		while ((p->str_cmd[*i] != ' ' && !(p->str_cmd[*i] == '>' &&
		!to_escape) && !(p->str_cmd[*i] == '<' && !to_escape) &&
		!(p->str_cmd[*i] == '|' && !to_escape)) && *i < len)
		{
			if (p->str_cmd[*i] == '\\' && !to_escape)
				to_escape = 1;
			else
				to_escape = 0;
			*i = *i + 1;
		}
	}
	while (p->str_cmd[*i] == ' ')
		*i = *i + 1;
}

static int		fill_redirection(t_parse_cmd *p, int *i, int len)
{
	int							in_out;
	t_redirection_type		 	red_type;

	in_out = 0;
	red_type = NONE;
	determine_red_type(p, i, &red_type, &in_out);
	while (1)
	{
		while (p->str_cmd[*i] == ' ')
			*i = *i + 1;
		if (red_type != 0)
		{
			p->buf = malloc(sizeof(char) *
			(size_component_formated(*p, *i, len) + 1));
			if (!p->buf)
				return (0);
			fill_buf(p, len, *i);
			add_red(p, p->buf, in_out, red_type);
		}
		red_type = NONE;
		skip_char_red(p, i, len);
		if (!determine_red_type(p, i, &red_type, &in_out))
			break ;
	}
	return (1);
}

int		check_between_args(t_parse_cmd *p, int *i, int len)
{
	while (p->str_cmd[*i] == ' ')
		*i = *i + 1;
	if (p->str_cmd[*i] == '<' || p->str_cmd[*i] == '>')
		fill_redirection(p, i, len);
	if (p->str_cmd[*i] == '|' || p->str_cmd[*i] == '\0' || *i >= len)
		return (1);
	return (0);
}

static char	**fill_args(t_parse_cmd *p, int *i, int len, int arg_nb)
{
	char		**args;
	int			size;
	int			j;

	args = init_args(arg_nb, *p);
	if (!args)
		return (0);
	j = 1;
	while (*i < len)
	{
		if (check_between_args(p, i, len))
			return (args);
		size = size_component_formated(*p, *i, len);
		p->buf = malloc(sizeof(char) * size + 1);
		if (!p->buf)
			return (NULL);
		fill_buf(p, len, *i);
		args[j] = p->buf;
		p->buf = NULL;
		*i = *i + real_component_size(*p, *i, len);
		j++;
	}
	return (args);
}

static int	set_previous_pipe(t_parse_cmd *p, int *i)
{
	if (p->p_cmd)
		p->p_cmd->pipe = p->cmd;
	else
	{
		if (p->str_cmd[*i] != '|')
			return (0);
	}
	p->p_cmd = p->cmd;
	*i = *i + 1;
	return (1);
}

static char	**parse_arguments(int *i, int size, int len, t_parse_cmd *p)
{
	int		arg_nb;
	char	**args;

	*i = *i + size;
	arg_nb = count_arg(p->str_cmd, *i, len);
	args = fill_args(p, i, len, arg_nb);
	if (!args)
		return (NULL);
	return (args);
}

static void	print_cmds(t_cmd *p)
{
	int	i;

	while (p != NULL)
	{
		printf("Cmd = %p\n", p);
		printf("Exe = %s\n", p->exe);
		i = 0;
		while (p->args[i] != NULL)
		{
			printf("Arg #%d: %s\n", i, p->args[i]);
			i++;
		}
		i = 0;
		while (p->in_redirection != NULL)
		{
			printf("In red #%d: type = %d path_or_endstr = %s \n",
			i, p->in_redirection->type, p->in_redirection->path_or_endstr);
			p->in_redirection = p->in_redirection->next;
			i++;
		}
		i = 0;
		while (p->out_redirection != NULL)
		{
			printf("Out red #%d: type = %d path_or_endstr = %s \n",
			i, p->out_redirection->type, p->out_redirection->path_or_endstr);
			p->out_redirection = p->out_redirection->next;
			i++;
		}
		p = p->pipe;
	}
}

t_cmd	*free_cmd_and_content(t_cmd *cmd)
{
	free_cmd_content(cmd, cmd);
	free_cmd(cmd);
	return (NULL);
}

t_cmd	*init_cmd(t_parse_cmd *p, int *i)
{
	t_cmd	*cmd;

	cmd = malloc(sizeof(t_cmd) * (1));
	if (!cmd)
	{
		p->cmd = 0;
		return (0);
	}
	if (*i == 0)
		p->first_cmd = cmd;
	while (p->str_cmd[*i] == ' ')
		*i = *i + 1;
	p->cmd = cmd;
	p->cmd->exe = NULL;
	p->cmd->args = NULL;
	p->cmd->pipe = NULL;
	p->cmd->in_redirection = NULL;
	p->cmd->out_redirection = NULL;
	return (cmd);
}

static int	init_cmd_exe(int *i, t_parse_cmd *p, int len, int *size)
{
	t_cmd	*cmd;

	cmd = init_cmd(p, i);
	if (!cmd)
		return (0);
	*size = size_component_formated(*p, *i, len);
	p->buf = malloc(sizeof(char) * (*size + 1));
	*size = real_component_size(*p, *i, len);
	if (!p->buf)
	{
		free(cmd);
		return (0);
	}
	fill_buf(p, len, *i);
	p->cmd->exe = p->buf;
	p->buf = NULL;
	while (p->str_cmd[*i] == ' ')
		*i = *i + 1;
	return (1);
}

static void	init_parsing(t_parse_cmd *p, char const *str_cmd, char **envp)
{
	p->p_cmd = NULL;
	p->first_cmd = NULL;
	p->str_cmd = str_cmd;
	p->envp = envp;
}

t_cmd	*parse_cmd(char const *str_cmd, int len, char **envp[])
{
	int				i;
	int				size;
	t_parse_cmd		p;

	i = 0;
	init_parsing(&p, str_cmd, *envp);
	while (1)
	{
		if (!init_cmd_exe(&i, &p, len, &size))
			return (NULL);
		p.cmd->args = parse_arguments(&i, size, len, &p);
		if (!p.cmd->args)
		{
			free_cmd_content(p.first_cmd, p.first_cmd);
			free_cmd(p.first_cmd);
			return (NULL);
		}
		if (!set_previous_pipe(&p, &i) || i > len)
			break ;
	}
	print_cmds(p.first_cmd);
	return (p.first_cmd);
}
