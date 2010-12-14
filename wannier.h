#ifndef WANNIER_H_
#define WANNIER_H_

#include "vecr.h"
#include <stdio.h>
#include <string>


namespace md_files {

	class WannierFile : public CoordinateFile {

		public:
			WannierFile (std::string wannierpath);
			~WannierFile ();

			// Various control functions
			void LoadNext ();
	};

}
#endif