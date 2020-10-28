/*
 * ProcessorBase.cpp
 *
 */

#include "ProcessorBase.hpp"

void ProcessorBase::setup_redirection(int my_num, int thread_num,
		OnlineOptions& opts)
{
    if (not opts.cmd_private_output_file.empty())
    {
        const string stdout_filename = get_parameterized_filename(my_num,
                thread_num, opts.cmd_private_output_file);
        stdout_redirect_file.open(stdout_filename.c_str(), ios_base::out);
    }
}
