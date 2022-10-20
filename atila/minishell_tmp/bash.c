/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bash.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosta-a <acosta-a@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/16 07:44:08 by acosta-a          #+#    #+#             */
/*   Updated: 2022/10/05 15:26:33 by acosta-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	signal_handler_bash(int sig)
{
	if (sig == SIGINT)
	{
		write(2, "\n", 1);
		exit(0);
	}
}

char	*preper_path(t_data **data, t_cursors *crs, char *path, char cmd[])
{
	if (!ft_memcmp((*data)->cmds[0][0], "./", 2))
		crs->i = 2;
	else if (!ft_memcmp((*data)->cmds[0][0], "../", 3))
		crs->i = 3;
	crs->flag = crs->i;
	while ((*data)->cmds[0][0][crs->i])
	{
		cmd[crs->j] = (*data)->cmds[0][0][crs->i];
		crs->i++;
		crs->j++;
	}
	cmd[crs->j] = '\0';
	return (path);
}

char	*ret_path(t_cursors *crs, char *path, char cmd[])
{
	crs->len = ft_strlen(path);
	if (crs->flag == 3)
	{
		while (crs->flag < 4 || crs->len == 0)
		{
			if (path[crs->len] == '/')
				crs->flag++;
			crs->len--;
		}
		path[crs->len + 1] = '\0';
	}
	path = ft_strjoin_2(path, "/");
	path = ft_strjoin_2(path, cmd);
	return (path);
}

void	exec_bash(t_data **data, char *path, char *args[])
{
	int			status;
	pid_t		pid;

	pid = fork();
	if (pid == 0)
	{
		signal(SIGINT, signal_handler_bash);
		if (execve(path, args, (*data)->envp)  == -1)
			exec_error_msg(path);
	}
	else
	{
		waitpid(pid, &status, 0);
		return ;
	}
}

void	ft_bash(t_data **data)
{
	char		cmd[256];
	char		tmp[256];
	char		*args[256];
	char		*path;
	t_cursors	*crs;

	init_crs(&crs);
	path = getcwd(tmp, sizeof(tmp));
	path = preper_path(data, crs, path, cmd);
	path = ret_path(crs, path, cmd);
	while(++crs->l < 257)
		args[crs->l] = NULL;
	args[0] = path;
	while((*data)->cmds[0][++crs->w])
		args[crs->w] = (*data)->cmds[0][crs->w];
	args[crs->w] = NULL;
	exec_bash(data, path, args);
	free(crs);
}