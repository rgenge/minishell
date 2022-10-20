/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   delimiters.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acosta-a <acosta-a@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/16 07:44:08 by acosta-a          #+#    #+#             */
/*   Updated: 2022/10/09 02:14:16 by acosta-a         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	*pathexec(char *cmd, char *envp[])
{
	char	**paths;
	char	*path;
	int		i;

	i = 0;
	while (envp[i] && ft_strncmp(envp[i], "PATH=", 5))
		i++;
	if (!envp[i])
		return (cmd);
	paths = ft_split(envp[i] + 5, ':');
	i = 0;
	while (paths[i])
	{
		path = ft_strjoin(paths[i], "/");
		path = ft_strjoin(path, cmd);
		if (access(path, F_OK) == 0)
			return (path);
		free(path);
		i++;
	}
	i = -1;
	free(paths);
	return (0);
}

char	*cmd_space_substitution(char *argv)
{
	int	i;

	i = 0;
	while (argv[i])
	{
		if (argv[i] == '\'' || argv[i] == '\"')
		{
			i++;
			if (argv[i] == '\'' || argv[i] == '\"')
				i++;
			while (argv[i] != '\'' && argv[i] != '\"' && argv[i])
			{
				if (argv[i] == ' ')
					argv[i] = 1;
				i++;
			}
		}
		i++;
	}
	if (argv[i] != '\0')
		argv[i] = '\0';
	return (argv);
}

char	**cmd_one_substitution(char **cmd)
{
	int		i;
	int		x;

	i = 0;
	while (cmd[i])
	{
		if (cmd[i][0] == '\'' || cmd[i][0] == '\"')
		{
			x = 1;
			if (cmd[i][x] == '\'' || cmd[i][x] == '\"')
				x++;
			while ((cmd[i][x] != '\'' && cmd[i][x] != '\"' ) && cmd[i][x])
			{
				if (cmd[i][x] == 1)
				cmd[i][x] = ' ';
				x++;
			}
			if (cmd[i][x + 1] != '\0' )
				x++;
			one_substitution_2(cmd, i, x);
		}
		i++;
	}
	return (cmd);
}

void	one_substitution_2(char **cmd, int i, int x)
{
	char	*tmp;

	tmp = ft_substr(cmd[i], 1, x - 1);
	free(cmd[i]);
	cmd[i] = tmp;
}

void	path_error(char *path, char **cmd)
{
	int		i;

	i = -1;
	if (!path)
	{
		while (cmd[i++])
			free(cmd[i]);
		free(cmd);
	}
}

void	execute(char *argv, t_data **data)
{
	char	**cmd;
	char	*path;

	cmd_space_substitution(argv);
	cmd = ft_split(argv, ' ');
	cmd_one_substitution (cmd);
	if (cmd[0] == NULL)
	{
		free(cmd[0]);
		free(cmd);
	}
	path = pathexec(cmd[0], (*data)->envp);
	if (execve(path, cmd, (*data)->envp)  == -1)
		exit(exec_error_msg(argv));
}

void	execute_pipe(char *argv, t_data **data)
{
	char	**cmd;
	char	*path;
	pid_t	pid;
	int		status;

	signal(SIGINT, child_signal_handler);
	pid = fork();
	if (pid == 0)
	{
	cmd_space_substitution(argv);
	cmd = ft_split(argv, ' ');
	cmd_one_substitution (cmd);
	if (cmd[0] == NULL)
	{
		free(cmd[0]);
		free(cmd);
	}
	path = pathexec(cmd[0], (*data)->envp);
	if (execve(path, cmd, (*data)->envp)  == -1)
		exit(exec_error_msg(argv));
	}
	waitpid(pid, &status, 0);
}

void	ft_pipe(t_data **data, int i, int flag, t_cursors *crs)
{
	int		pipefd[2];
	pid_t	pid;
	int		status;

	signal(SIGINT, child_signal_handler);
	pipe (pipefd);
	pid = fork();
	if (pid == 0)
	{
		close (pipefd[IN]);
		dup2 (pipefd[OUT], STDOUT);
		if (flag == 8)
		{
			crs->flagpipe = 0;
			ft_input(data, crs);
			crs->flagpipe = 1;
		}
		else
			builtin_execute(data, i, flag, crs);

	}
	waitpid(pid, &status, 0);
	close(pipefd[OUT]);
	dup2(pipefd[IN], STDIN);
	if (crs->flagpipe == 1)
		exit(0);
}
void	ft_output(t_data **data, t_cursors *crs)
{
	pid_t	pid;

	pid = fork();
	if (pid == 0)
	{
	int i = 1;
	if(ft_strncmp((*data)->cmds[crs->i2][crs->j2], ">", 2) == 0)
		while (i < crs->k2 && (*data)->cmds[crs->i2 + i] && ft_strncmp((*data)->cmds[crs->i2 + i][0], ">", 2) == 0)
		{
			crs->output = open((*data)->cmds[crs->i2 + i][1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
			i++;
		}
	if (crs->output == -1)
		{
			ft_putstrs("bash:", (*data)->cmds[crs->i2][1],
			": Permission denied\n" , STDERR);
			(*data)->exit_return = 1;
			return ;
		}
	ft_output_2(data, crs);
	}
	waitpid(pid, &crs->status, 0);
}

void	ft_output_2(t_data **data, t_cursors *crs)
{
		crs->saved_stdout = dup(STDOUT);
		dup2(crs->output, STDOUT);
		close(crs->output);
		builtin_execute(data, crs->i2, crs->flag, crs);
		dup2(crs->saved_stdout, STDOUT);
		close(crs->saved_stdout);
}


void	ft_input(t_data **data, t_cursors *crs)
{
	pid_t	pid;
	pid_t	pid2;

	if (!ft_strncmp((*data)->cmds[crs->i2][crs->j2], "<", 2) && !ft_strncmp
			((*data)->cmds[crs->i2 + 1][0], "<", 1))
	{
		crs->flag = 1;
		crs->j2 = 1;
		ft_here_doc(data, crs);
		return ;
	}
		if (crs->flagpipe == 8 && (*data)->cmds[crs->i2 + 1] && (*data)->cmds
			[crs->i2 + 1][1] && !ft_strncmp((*data)->cmds[crs->i2 +
			1][1], "|", 2))
		{
//			crs->flagpipe = 0;
			ft_pipe(data, crs->i2, crs->flagpipe, crs);
			crs->i2+=2;
		}
		if (((*data)->cmds[crs->i2][0] && !(*data)->cmds[crs->i2][1] ) || ((*data)->cmds[crs->i2][1] && ft_strncmp((*data)->cmds[crs->i2]
			[1], "<", 2))){
			builtin_execute(data, crs->i2, 1, crs);
//			exit(0);
			}
		while((*data)->cmds[crs->i2 + 1] && (*data)->cmds
			[crs->i2 + 1][1] && !ft_strncmp((*data)->cmds[crs->i2 +
			1][1], "<", 2))
		{
			crs->i2++;
			pid2 = fork();
			if (pid2 == 0)
			{
			crs->i2--;
			(*data)->cmds[crs->i2 + 1][0] = ft_clean_quotes((*data)->cmds[crs->i2 + 1][0], '\'');
			crs->input = open((*data)->cmds[crs->i2 + 1][0], O_RDONLY, S_IRWXU);
			if (crs->input == -1)
			{
				char	cwd[4096];
				char	*path;

				getcwd(cwd, 4096);
				path = ft_strdup(cwd);
				path = ft_strjoin_2(cwd, "/");
				path = ft_strjoin_2(path, (*data)->cmds[crs->i2 + 1][0]);
				input_error_msg(path);
			}
			if (crs->input > 0)
			{
				char	*cmd3;

				crs->saved_stdin = dup(STDIN);
				dup2(crs->input, STDIN);
				cmd3 = ft_strdup((*data)->cmds[0][0]);
				cmd3 = ft_strjoin_2(cmd3, " ");
				cmd3 = ft_strjoin_2(cmd3, (*data)->cmds[crs->i2 + 1][0]);
//				execute(cmd3, data);
				execute_pipe(cmd3, data);
//				builtin_execute(data, crs->i2, crs->flag, crs);
				dup2(crs->saved_stdin, STDIN);
				close(crs->saved_stdin);
				close(crs->input);
			}
//		waitpid(pid, &crs->status, 0);
//			builtin_execute(data, crs->i2, 1, crs);
//			ft_pipe(data, crs->i2, crs->flagpipe, crs);
			}
			crs->i2++;
			waitpid(pid2, &crs->status, 0);
			crs->flagpipe = 10;
		}
	if(crs->flagpipe == 10)
		return ;
	pid = fork();
	if (pid == 0)
	{
	/*	if ((*data)->cmds[crs->i2][2] && ft_strncmp((*data)->cmds[crs->i2]
			[2], "<", 2)){
			builtin_execute(data, crs->i2, crs->flagecho, crs);
			exit(0);
			}*/
		if (((*data)->cmds[crs->i2][2] && !ft_strncmp((*data)->cmds[crs->i2][2], "<", 2) && ft_strncmp((*data)->cmds[crs->i2 + 1][0], "<", 2)) || (!(*data)->cmds[crs->i2][2] && (*data)->cmds[crs->i2][1] && !ft_strncmp((*data)->cmds[crs->i2][1], "<", 2) && ft_strncmp((*data)->cmds[crs->i2 + 1][0], "<", 2)))
		{
			(*data)->cmds[crs->i2 + 1][0] = ft_clean_quotes((*data)->cmds[crs->i2 + 1][0], '\'');
			crs->input = open((*data)->cmds[crs->i2 + 1][0], O_RDONLY, S_IRWXU);
		}
		if (((*data)->cmds[crs->i2 + 2] && (*data)->cmds[crs->i2 + 2][0] && !ft_strncmp((*data)->cmds[crs->i2 + 2][0], ">", 2)) || ((*data)->cmds[crs->i2 + 1] && (*data)->cmds[crs->i2 + 1][1] && !ft_strncmp((*data)->cmds[crs->i2 + 1][1], ">", 2))){
			ft_in_output(data, crs);
			return ;}
		if (crs->input == -1)
		{/*
			ft_putstrs("bash:", (*data)->cmds[crs->i2][1],
			": No such file or directory\n" , STDERR);
			exit(1);
		*/
			char	cwd[4096];
			char	*path;

			getcwd(cwd, 4096);
			path = ft_strdup(cwd);
			path = ft_strjoin_2(cwd, "/");
			path = ft_strjoin_2(path, (*data)->cmds[crs->i2 + 1][0]);
			input_error_msg(path);
//			waitpid(pid, &crs->status, 0);
		}
		if (crs->input > 0)
		{
			crs->saved_stdin = dup(STDIN);
			dup2(crs->input, STDIN);
			builtin_execute(data, crs->i2, crs->flag, crs);
			dup2(crs->saved_stdin, STDIN);
			close(crs->saved_stdin);
			close(crs->input);
		}
//	waitpid(pid, &crs->status, 0);
	}
	waitpid(pid, &crs->status, 0);
	if ((*data)->cmds[crs->i2 + 1] && (*data)->cmds[crs->i2 + 1][1] && (*data)->cmds[crs->i2 + 1][1][0] == '>')
	{
		crs->i2 += 3;
		crs->j2--;
	}
	else
		crs->i2+=2;
}

void	ft_in_output(t_data **data, t_cursors *crs)
{
		if (!ft_strncmp((*data)->cmds[crs->i2 + 1][crs->j2], ">", 2) && !ft_strncmp
			((*data)->cmds[crs->i2 + 2][0], ">", 2))
			crs->output = open((*data)->cmds[crs->i2 + 2][1], O_CREAT
					| O_WRONLY | O_APPEND, S_IRWXU);
		else if (!ft_strncmp((*data)->cmds[crs->i2 + 1][crs->j2], ">", 2) && ft_strncmp
			((*data)->cmds[crs->i2 + 2][0], ">", 2))
		crs->output = open((*data)->cmds[crs->i2 + 2][0], O_CREAT | O_WRONLY
					| O_TRUNC, S_IRWXU);
		if (crs->output == -1)
		{
			ft_putstrs("bash:", (*data)->cmds[crs->i2][1],
			": No such file or directory" , STDERR);
			(*data)->exit_return = 1;
			return ;
		}
		crs->saved_stdin = dup(STDIN);
		dup2(crs->input, STDIN);
		crs->saved_stdout = dup(STDOUT);
		dup2(crs->output, STDOUT);
		builtin_execute(data, crs->i2, crs->flag, crs);
		dup2(crs->saved_stdout, STDOUT);
		dup2(crs->saved_stdin, STDIN);
		close(crs->saved_stdin);
		close(crs->input);
		close(crs->output);
		close(crs->saved_stdout);
}

