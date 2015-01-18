/* itq: Frontend program to convert .it to .itq and vice versa
 *
 * Usage:
 *   itq encode source.it
 *   itq decode source.itq
 *
 * Copyright 2011-2014 Eric Gregory and Stevie Hryciw
 *
 * Modipulate.
 * https://github.com/MrEricSir/Modipulate/
 *
 * Modipulate is released under the BSD license.  See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <libopenmpt.hpp>

#define USAGE \
"Usage:\n" \
"  itq encode source.it        compress .it file into .itq\n" \
"  itq decode source.itq       decompress .itq file into .it\n" \
"Currently, encoding is restricted to Ogg Vorbis with qualtiy 2 (-q2).\n"

#define ARG_CMD argv[1]
#define ARG_FILE argv[2]
#define OUTPUT_IT 1
#define OUTPUT_ITQ 2

int main(int argc, char** argv)
{
	int action = 0;
	openmpt::module* modfile;

	/* Check command line arguments */
	if (argc != 3)
	{
		printf(USAGE);
		return 1;
	}
	if (memcmp(ARG_CMD, "encode", 6) == 0)
		action = OUTPUT_ITQ;
	else if (memcmp(ARG_CMD, "decode", 6) == 0)
		action = OUTPUT_IT;
	else
	{
		printf("Error: Incorrect usage\n");
		printf(USAGE);
		return 1;
	}

	/* Read the source file */
	std::ifstream file(std::string(ARG_FILE), std::ios::binary);

    if (file.fail() || !file.good())
    {
		printf("Error: Could not open source file %s\n", ARG_FILE);
		return 1;
    }
    
    try
    {
	    modfile = new openmpt::module(file);
    }
    catch(const openmpt::exception& e)
    {
		printf("Error: Could not open source file %s\n", ARG_FILE);
		return 1;
    }

	/* Encode or decode and save new file */
	if (action == OUTPUT_ITQ)
	{
		printf("Encoding to out.itq ...\n");
		if (!modfile->save_itq("out.itq", 0.2f))
		{
			printf("Error: Problem writing output file\n");
			return 1;
		}
	}
	else if (action == OUTPUT_IT)
	{
		printf("Decoding to out.it ...\n");
		if (!modfile->save_it("out.it"))
		{
			printf("Error: Problem writing output file\n");
			return 1;
		}
	}
	else /* Should not happen */
	{
		printf("Something went wrong and I have no idea what!\n");
		return 1;
	}

	return 0;
}
