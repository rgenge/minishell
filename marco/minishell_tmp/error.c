/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosta-a <acosta-a@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/09 14:43:17 by mcesar-d          #+#    #+#             */
/*   Updated: 2022/10/27 15:19:19 by acosta-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	print_error(int e)
{
	char	*erros[] = {"OK", "unclosed quotes"};

	ft_putstr_fd("Error: ", 1);
	ft_putstr_fd(erros[e], 1);
	ft_putstr_fd("\n", 1);
}

int	exec_error_msg(char *path, t_data ** data)
{
	int	fd;
	DIR	*folder;

	fd = open(path, O_WRONLY);
	folder = opendir(path);
	ft_putstrs("minishell: ", path, NULL, STDERR);
	if (access(path, F_OK) != 0)
	{
		ft_putstr_fd(": command not found\n", STDERR);
		(*data)->exit_return = 127;
		return (127);//(nao precisa)
	}
	else if (fd == -1 && folder == NULL)
	{
		ft_putstr_fd ("No such file or directory\n", STDERR);
		(*data)->exit_return = 127;
		return (127);//(nao precisa)
	}
	else if (fd == -1 && folder != NULL)
		ft_putstr_fd (": is a directory\n", STDERR);
	else if (fd != -1 && folder == NULL)
		ft_putstr_fd (": Permission denied\n", STDERR);
	if (folder)
		closedir(folder);
	close(fd);
	(*data)->exit_return = 126;
	return (126);//(nao precisa)
}

int	input_error_msg(char *path, t_data **data, t_cursors *crs)
{
	int	fd;
	DIR	*folder;

	clean_all(data, crs);
	fd = open(path, O_WRONLY);
	folder = opendir(path);
	ft_putstr_fd("minishell: ", STDERR);
	if (fd == -1 && folder == NULL)
	{
		ft_putstr_fd ("No such file or directory\n", STDERR);
		free(path);
		(*data)->exit_return = 127;
		exit(127);//(nao precisa)
	}
	else if (fd != -1 && folder == NULL)
		ft_putstr_fd (": Permission denied\n", STDERR);
	if (folder)
		closedir(folder);
	close(fd);
	free(path);
	(*data)->exit_return = 126;
	exit (126);//(nao precisa)
}

int	input_error_msg_noexit(char *path, t_data **data, t_cursors *crs)
{
	int	fd;
	DIR	*folder;

	clean_all(data, crs);
	fd = open(path, O_WRONLY);
	folder = opendir(path);
	ft_putstr_fd("minishell: ", STDERR);
	if (fd == -1 && folder == NULL)
	{
		ft_putstr_fd ("No such file or directory\n", STDERR);
		free(path);
		(*data)->exit_return = 127;
		return (127);//(nao precisa)
	}
	else if (fd != -1 && folder == NULL)
		ft_putstr_fd (": Permission denied\n", STDERR);
	if (folder)
		closedir(folder);
	close(fd);
	free(path);
	(*data)->exit_return = 126;
	return (126);//(nao precisa)
}


int	error_msg(char *message)
{
	ft_putstr_fd(message, 2);
	ft_putstr_fd("\n", 2);
	exit(ERROR);
}
