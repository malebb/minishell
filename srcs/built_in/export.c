/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbrunet <clbrunet@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/06 19:04:47 by clbrunet          #+#    #+#             */
/*   Updated: 2021/03/06 19:04:50 by clbrunet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "built_in.h"

int	export_cmd(t_cmd const *cmd, char **envp_ptr[])
{
	(void)cmd;
	(void)envp_ptr;
	printf("salut\n");
	fflush(stdout);
	return (1);
}
