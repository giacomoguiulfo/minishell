/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   msh_exec.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gguiulfo <gguiulfo@student.42.us.org>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/05/05 21:57:23 by gguiulfo          #+#    #+#             */
/*   Updated: 2017/05/12 11:07:23 by gguiulfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <minishell.h>

char	**msh_cp_env(t_dnarr *newenvp)
{
	char	**ret;
	int		i;
	int		size;

	i = -1;
	size = 0;
	while (++i < newenvp->end)
		if (newenvp->contents[i] != NULL)
			size++;
	ret = (char **)ft_memalloc(sizeof(char *) * (size + 1));
	i = -1;
	size = 0;
	while (++i < newenvp->end)
	{
		if (newenvp->contents[i] != NULL)
			ret[size++] = newenvp->contents[i]; // TODO: Strdup or not?
	}
	return (ret);
}

void	msh_run_prog(char *executable, char **args, char **env)
{
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid == 0)
	{
		if (execve(executable, args, env) == -1)
		{
			ft_dprintf(2, "msh: permission denied for %s\n", executable);
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		ft_dprintf(2, "msh: unable to fork the process %d\n", pid);
		exit(EXIT_FAILURE);
	}
	wait(&status);
}

char	*msh_check_bin(char *executable, char *path)
{
	struct stat statbuf;
	char *ret;

	ft_asprintf(&ret, "%s/%s", path, executable);
	if (lstat(ret, &statbuf) == -1)
	{
		free(ret);
		return (NULL);
	}
	else if ((statbuf.st_mode & S_IFMT) == S_IFREG)
		return (ret);
	return (NULL);
}

int		msh_launch_bins(char **args, char **env)
{
	/* NEW PART */
	char	**bins;
	char	*executable;
	int		i;

	i = 0;
	// ft_printf("---launch bins---\n");
	bins = ft_strsplit(msh_get_env(env, "PATH"), ':');
	while (bins[i])
	{
		executable = msh_check_bin(args[0], bins[i]);
		// ft_printf("here executable: %s\n", executable);
		if (executable != NULL)
		{
			ft_free_map(bins);
			free(args[0]);
			args[0] = executable;
			// ft_printf("executable: %s\n", executable);
			msh_run_prog(executable, args, env);
			// free(executable);
			return (1);
		}
		else
			i++;
	}
	ft_free_map(bins);
	return (0);
	/* -------- */
	// pid_t	pid;
	// pid_t	wpid;
	// int		status;
	// char	*cmd;
	// char	**arr;
	// char	**env;
	// int		i = 0;
	//
	// env = msh_cp_env(newenvp); // ENV is just pointing, it doesn't allocate new memory
	// // for (int i = 0; envp[i]; i++)
	// // 	ft_printf("%s\n", envp[i]);
	// pid = fork();
	// if (pid == 0)
	// {
	// 	arr = ft_strsplit(msh_get_env(env, "PATH"), ':');
	// 	ft_asprintf(&cmd, "%s/%s", arr[i++], args[0]);
	// 	// ft_asprintf(&cmd, "/bin/%s", args[0]); // TODO: This is not good because I can't free cmd, I think...
	// 	// i++;
	// 	// TODO: Check access of paths for each path like this -> access(arr[i], X_OK)
	// 	while (execve(cmd, args, env) == -1)
	// 	{
	// 		ft_printf("cmd: [%s]\n", cmd);
	// 		ft_printf("arr[%d]: [%s]\n", i, arr[i]);
	// 		free(cmd);
	// 		if (arr[i])
	// 			ft_asprintf(&cmd, "%s/%s", arr[i++], args[0]);
	// 		else
	// 		{
	// 			break ;
	// 		}
	// 	}
	// 	if (execve(args[0], args, env) == -1)
	// 	// if (execve(args[0], args, envp) == -1) // This one is the rigth one
	// 	// if (execvp(args[0], args) == -1) // Not this one...
	// 	{
	// 		// ft_dprintf(2, "msh: command not found: %s\n", cmd);
	// 		ft_dprintf(2, "msh: command not found: %s\n", args[0]); // Same here
	// 	}
	// 	exit(EXIT_FAILURE); // TODO: We have to free stuff before exiting... or not?
	// }
	// else if (pid < 0)
	// 	ft_dprintf(2, "msh: unable to fork the process\n");
	// else
	// {
	// 	wpid = waitpid(pid, &status, WUNTRACED);
	// 	while (!WIFEXITED(status) && !WIFSIGNALED(status))
	// 		wpid = waitpid(pid, &status, WUNTRACED);
	// }
	// free(env);
	// return (1);
}

int		msh_launch_reg(char *prog, char **args, char **env)
{
	struct stat statbuf;

	// ft_printf("---launch regular---\n");
	if (lstat(prog, &statbuf) == -1)
		return (-1);
	msh_run_prog(prog, args, env);
	return (0);
}

int		msh_run_builtins(char **args, t_dnarr *newenvp, char **env)
{
	if (!ft_strcmp(args[0], "cd"))
		return (msh_cd(args, newenvp, env));
	else if (!ft_strcmp(args[0], "echo"))
		return (msh_echo(args, env));
	else if (!ft_strcmp(args[0], "exit") || !ft_strcmp(args[0], "EXIT"))
		return (msh_exit(args));
	else if (!ft_strcmp(args[0], "help"))
		return (msh_help(args));
	else if (!ft_strcmp(args[0], "env"))
		return (msh_displ_env(args, newenvp));
	else if (!ft_strcmp(args[0], "setenv"))
		return (msh_setenv(args, newenvp, env));
	else if (!ft_strcmp(args[0], "unsetenv"))
		return (msh_unsetenv(args, newenvp, env));
	return (-1);
}

int		msh_execute(char **args, t_dnarr *newenvp)
{
	int ret;
	char **env;

	if (args[0] == NULL)
		return (1);
	env = msh_cp_env(newenvp);
	ret = msh_run_builtins(args, newenvp, env);
	if (ret != -1)
	{
		free(env);
		return (ret);
	}
	if (msh_launch_reg(args[0], args, env) == -1)
	{
		if (msh_env_idx(newenvp, "PATH") == -1 || !msh_launch_bins(args, env))
			ft_dprintf(2, "msh: command not found: %s\n", args[0]);
	}
	free(env);
	return (1);
}
