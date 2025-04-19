#include <shell.h>
#include <sys/wait.h> // Required for waitpid [cite: 30]
#include <errno.h>    // Required for perror

/**
 * @brief Executes a single command, handling input/output redirection.
 *
 * @param cmd The command structure containing arguments and file descriptors.
 * @param pipe_in_fd File descriptor for reading from the previous command's pipe (if any).
 * @param pipe_out_fd File descriptor for writing to the next command's pipe (if any).
 * @param is_first Boolean indicating if this is the first command in a potential pipeline.
 * @param is_last Boolean indicating if this is the last command in a potential pipeline.
 */
void execute_single_command(command *cmd, int pipe_in_fd, int pipe_out_fd, int is_first, int is_last)
{
	pid_t pid = fork(); // Create a child process [cite: 6, 22, 33]

	if (pid < 0) // Check if fork failed [cite: 32]
	{
		perror("fork failed"); // Report error [cite: 32]
		exit(EXIT_FAILURE); // Exit if fork fails
	}
	else if (pid == 0) // Child process
	{
		// --- Input Redirection ---
		if (!is_first) // If not the first command, read from the incoming pipe
		{
			// Redirect standard input to the read end of the previous pipe
			if (dup2(pipe_in_fd, STDIN_FILENO) < 0) // [cite: 6, 32]
			{
				perror("dup2 pipe_in failed"); // Report error [cite: 32]
				exit(EXIT_FAILURE);
			}
			close(pipe_in_fd); // Close the original pipe file descriptor [cite: 31]
		}
		else if (cmd->fd_in != STDIN_FILENO) // Handle explicit input redirection (<)
		{
			// Redirect standard input to the specified input file descriptor
			if (dup2(cmd->fd_in, STDIN_FILENO) < 0) // [cite: 6, 32]
			{
				perror("dup2 fd_in failed"); // Report error [cite: 32]
				exit(EXIT_FAILURE);
			}
			close(cmd->fd_in); // Close the original input file descriptor [cite: 25]
		}

		// --- Output Redirection ---
		if (!is_last) // If not the last command, write to the outgoing pipe
		{
			// Redirect standard output to the write end of the next pipe
			if (dup2(pipe_out_fd, STDOUT_FILENO) < 0) // [cite: 6, 32]
			{
				perror("dup2 pipe_out failed"); // Report error [cite: 32]
				exit(EXIT_FAILURE);
			}
			close(pipe_out_fd); // Close the original pipe file descriptor [cite: 31]
		}
		else if (cmd->fd_out != STDOUT_FILENO) // Handle explicit output redirection (>, >>)
		{
			// Redirect standard output to the specified output file descriptor
			if (dup2(cmd->fd_out, STDOUT_FILENO) < 0) // [cite: 6, 32]
			{
				perror("dup2 fd_out failed"); // Report error [cite: 32]
				exit(EXIT_FAILURE);
			}
			close(cmd->fd_out); // Close the original output file descriptor [cite: 25]
		}

		// Execute the command
		execvp(cmd->args[0], cmd->args); // [cite: 33]
		// If execvp returns, an error occurred
		perror("execvp failed"); // Report error [cite: 32]
		exit(EXIT_FAILURE); // Exit child process on execvp error
	}
	else // Parent process
	{
		// Parent doesn't need the command's specific fds, they are duplicated in child or handled by pipes
		if (cmd->fd_in != STDIN_FILENO)
		{
			close(cmd->fd_in); // Close input fd in parent if it was opened [cite: 25]
		}
		if (cmd->fd_out != STDOUT_FILENO)
		{
			close(cmd->fd_out); // Close output fd in parent if it was opened [cite: 25]
		}
		// Store the pid in the command structure (optional, but might be useful)
		// cmd->pid = pid; // Example if you add pid to command struct
	}
}

/**
 * @brief Executes a list of commands, handling pipelines.
 *
 * @param command_list A linked list where each node contains a 'command' struct.
 */
void executor(List *command_list)
{
	printf("____EXECUTOR____\n"); // Indicate executor start
	// display(command_list, display_command); // Display parsed command (optional debug)

	Node *current = command_list->head; // Start at the head of the command list
	int num_commands = 0; // Counter for the number of commands
	while (current != NULL) // Count commands in the list
	{
		num_commands++; // Increment command count
		current = current->next; // Move to the next node
	}

	if (num_commands == 0) // If there are no commands, do nothing
	{
		return; // Exit the function
	}

	int pipe_fds[2]; // Array to hold file descriptors for a pipe [cite: 20, 31]
	int prev_pipe_read_end = -1; // Initialize previous pipe read end to -1 (no previous pipe)
	pid_t pids[num_commands]; // Array to store child process IDs
	int status; // Variable to store child exit status

	current = command_list->head; // Reset current to the head of the list
	for (int i = 0; i < num_commands; i++) // Loop through each command
	{
		command *cmd = (command *)current->content; // Get the command structure from the node
		int is_first = (i == 0); // Check if it's the first command
		int is_last = (i == num_commands - 1); // Check if it's the last command
		int current_pipe_write_end = -1; // Initialize current pipe write end

		if (!is_last) // If it's not the last command, create a pipe for the next command [cite: 20]
		{
			/* Pipe explanation: pipe() creates a unidirectional communication channel.
			 * pipe_fds[0] becomes the read end, pipe_fds[1] becomes the write end.
			 * Data written to pipe_fds[1] can be read from pipe_fds[0].
			 * We use this to connect the standard output of one command
			 * to the standard input of the next command in the pipeline. [cite: 21]
			 */
			if (pipe(pipe_fds) < 0) // Create the pipe [cite: 31]
			{
				perror("pipe failed"); // Report error [cite: 32]
				exit(EXIT_FAILURE); // Exit if pipe creation fails
			}
			current_pipe_write_end = pipe_fds[1]; // Store the write end for the current command
		}

		// --- Fork and Execute ---
		pid_t pid = fork(); // Create a child process for the current command [cite: 6, 22, 33]

		if (pid < 0) // Check if fork failed
		{
			perror("fork failed"); // Report error [cite: 32]
			// Consider more robust error handling (e.g., cleanup previous children)
			exit(EXIT_FAILURE); // Exit if fork fails
		}
		else if (pid == 0) // Child process
		{
			// --- Input Redirection (Child) ---
			if (!is_first) // If not the first command, connect to the previous pipe's read end
			{
				if (dup2(prev_pipe_read_end, STDIN_FILENO) < 0) // Redirect stdin [cite: 6, 32]
				{
					perror("child dup2 prev_pipe_read_end failed"); // Report error [cite: 32]
					exit(EXIT_FAILURE);
				}
				close(prev_pipe_read_end); // Close the original descriptor [cite: 25, 31]
			}
			else if (cmd->fd_in != STDIN_FILENO) // Handle explicit input redirection (<) only for the first command
			{
				if (dup2(cmd->fd_in, STDIN_FILENO) < 0) // Redirect stdin [cite: 6, 32]
				{
					perror("child dup2 fd_in failed"); // Report error [cite: 32]
					exit(EXIT_FAILURE);
				}
				close(cmd->fd_in); // Close the original descriptor [cite: 25]
			}

			// --- Output Redirection (Child) ---
			if (!is_last) // If not the last command, connect to the current pipe's write end
			{
				if (dup2(current_pipe_write_end, STDOUT_FILENO) < 0) // Redirect stdout [cite: 6, 32]
				{
					perror("child dup2 current_pipe_write_end failed"); // Report error [cite: 32]
					exit(EXIT_FAILURE);
				}
				close(current_pipe_write_end); // Close the original descriptor [cite: 25, 31]
                // Also close the read end of the *current* pipe in the child, it's not needed here
                close(pipe_fds[0]); // [cite: 25, 31]
			}
			else if (cmd->fd_out != STDOUT_FILENO) // Handle explicit output redirection (>, >>) only for the last command
			{
				if (dup2(cmd->fd_out, STDOUT_FILENO) < 0) // Redirect stdout [cite: 6, 32]
				{
					perror("child dup2 fd_out failed"); // Report error [cite: 32]
					exit(EXIT_FAILURE);
				}
				close(cmd->fd_out); // Close the original descriptor [cite: 25]
			}

            /* Fork behavior for "cat | cat | ls": [cite: 24]
             * 1. Parent forks 'cat' (1). Pipe A created.
             * 2. Parent forks 'cat' (2). Pipe B created. Parent closes Pipe A write end. Child 1 closes Pipe A read end.
             * 3. Parent forks 'ls'. Parent closes Pipe B write end. Child 2 closes Pipe B read end.
             * 4. Child 1 ('cat') writes to Pipe A, waits for input (stdin).
             * 5. Child 2 ('cat') reads from Pipe A, writes to Pipe B, waits for input from Pipe A.
             * 6. Child 3 ('ls') reads from Pipe B, executes, writes to stdout, and exits. Output of ls appears first.
             * 7. Child 3 exiting closes the write end of Pipe B.
             * 8. Child 2 ('cat') reading from Pipe A finishes (if Child 1 closes Pipe A), attempts to write to Pipe B. Since Child 3 (reader) exited, writing to Pipe B results in SIGPIPE, terminating Child 2.
             * 9. Child 2 exiting closes the read end of Pipe A.
             * 10. Child 1 ('cat') reading from stdin finishes (e.g., on EOF from terminal after Enter hits), attempts to write to Pipe A. Since Child 2 (reader) exited, writing to Pipe A results in SIGPIPE, terminating Child 1.
             * 11. The terminal waits for input because the first 'cat' (Child 1) is attached to the terminal's stdin. Hitting Enter twice might send EOF or signals causing the remaining 'cat' processes to terminate due to SIGPIPE as described.
            */

			// --- Execute Command ---
			execvp(cmd->args[0], cmd->args); // Execute the command [cite: 33]
			// If execvp returns, it failed
			perror(cmd->args[0]); // Report error specific to the command [cite: 32]
			exit(EXIT_FAILURE); // Exit child with failure status
		}
		else // Parent process
		{
			pids[i] = pid; // Store the child PID

			// --- Close Parent's Pipe Ends ---
			if (!is_first) // If not the first command
			{
				close(prev_pipe_read_end); // Close the read end of the previous pipe in the parent [cite: 25, 31]
			}
			if (!is_last) // If not the last command
			{
				close(current_pipe_write_end); // Close the write end of the current pipe in the parent [cite: 25, 31]
				prev_pipe_read_end = pipe_fds[0]; // The read end of the current pipe becomes the previous read end for the next iteration
			}

            // --- Close Command Specific FDs in Parent ---
            // These might have been opened by the parser for redirection (<, >)
            // We close them here after the child has potentially duplicated them.
            /* File Descriptor Closing Explanation: [cite: 26]
             * File descriptors opened by the parser for redirection (cmd->fd_in, cmd->fd_out)
             * need to be closed in the parent process after the child process has been forked.
             * The child process duplicates these descriptors if needed (using dup2) and then closes
             * its copy of the original descriptor. The parent closes its original copy here.
             * Pipe file descriptors are also explicitly closed in both parent and child
             * processes as soon as they are no longer needed. The write end of a pipe should be
             * closed in the parent and any child that isn't writing to it. The read end should be
             * closed in the parent and any child that isn't reading from it. Standard input (0),
             * output (1), and error (2) are typically not closed unless intentionally redirected.
             * Closing descriptors prevents resource leaks and ensures correct pipeline behavior (e.g., EOF propagation).
             */
            if (cmd->fd_in != STDIN_FILENO) {
                 close(cmd->fd_in); // [cite: 25]
            }
            if (cmd->fd_out != STDOUT_FILENO) {
                 close(cmd->fd_out); // [cite: 25]
            }

			current = current->next; // Move to the next command in the list
		}
	}

	// --- Wait for all children ---
	current = command_list->head; // Reset current to the head for storing status
	for (int i = 0; i < num_commands; i++) // Loop through the number of commands launched
	{
		command *cmd = (command *)current->content; // Get the command structure
		if (waitpid(pids[i], &status, 0) < 0) // Wait for the specific child process [cite: 6, 18, 33]
		{
			perror("waitpid failed"); // Report error [cite: 32]
		}
		else
		{
			if (WIFEXITED(status)) // Check if the child terminated normally
			{
				cmd->exit_status = WEXITSTATUS(status); // Store the exit status [cite: 18]
			}
			else if (WIFSIGNALED(status)) // Check if the child was terminated by a signal
            {
                // fprintf(stderr, "Process %d terminated by signal %d\n", pids[i], WTERMSIG(status)); // Optional debug
                cmd->exit_status = 128 + WTERMSIG(status); // Convention for signal exits
            }
			else
			{
				cmd->exit_status = -1; // Indicate unknown exit status
			}
		}
		current = current->next; // Move to the next node
	}

	// display(command_list, display_exit_status); // Display exit statuses (optional debug)
	check_open_fds(); // Check for open file descriptors (debug) [cite: 25]

    // Note: Memory leak checking should be done using Valgrind as specified [cite: 19]
    // valgrind --leak-check=full ./itush
    // The destroy function called after executor in handle_tokens should free allocated memory. [cite: 29]
}