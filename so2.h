#ifndef SO2_H_
#define SO2_H_

#include "molecule.h"

namespace md_system {


	class SulfurDioxide : public Molecule {

		public:
			SulfurDioxide ();
			SulfurDioxide (const Molecule& mol);
			SulfurDioxide (const MolPtr& mol);
			~SulfurDioxide () { }

			void SetAtoms ();

			AtomPtr S() const { return _s; }
			AtomPtr O1() const { return _o1; }
			AtomPtr O2() const { return _o2; }

			VecR SO1 () const { return _so1; }
			VecR SO2 () const { return _so2; }

			virtual VecR ReferencePoint () const { return _s->Position(); }
			virtual void SetOrderAxes ();

			double Angle () const { return (_so1 < _so2); } // cos of the O-S-O angle
			VecR Bisector ();


		private:
			AtomPtr	_s, _o1, _o2;
			VecR	_so1, _so2;

	}; // Sulfur Dioxide


}
#endif
