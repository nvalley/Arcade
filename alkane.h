#ifndef ALKANE_H_
#define ALKANE_H_

#include "molecule.h"

namespace alkane {
	using namespace md_system;


	class Alkane : public Molecule {

		public:
			Alkane ();			// a default constructor
			virtual ~Alkane ();
			Alkane (const Molecule& molecule);		// copy constructor for casting from a molecule

			static int numAlkanes;			// total number of carbon chains in the system

			virtual VecR ReferencePoint () const { 
				return this->CenterOfMass(); 
			}

			virtual void SetAtoms () { }

			// Functions for analysis
			//virtual void SetAtoms () = 0;
			ThreeAtomGroup& COO1 () { return carbonyl_1; }
			ThreeAtomGroup& COO2 () { return carbonyl_2; }
			AtomPtr C1 () const { return c1; }	// first carbonyl
			AtomPtr C2 () const { return c2; }	// second carbonyl 
			AtomPtr H1 () const { return h1; }	// the two acid protons
			AtomPtr H2 () const { return h2; }
			AtomPtr O1 () const { return o1; }	// carbonyl oxy's
			AtomPtr O2 () const { return o2; }
			AtomPtr OH1 () const { return oh1; }	// alcohol oxys
			AtomPtr OH2 () const { return oh2; }
			
		protected:
			ThreeAtomGroup carbonyl_1, carbonyl_2;
			AtomPtr c1,  c2;		// carbonyl carbons
			AtomPtr oh1, oh2;		// alcohol oxygens
			AtomPtr o1,  o2;		// carbonyl oxygens
			AtomPtr h1,  h2;
	};




	class MalonicAcid : public Alkane {

		private:
			// middle carbon and hydrogens
			AtomPtr cm;
			AtomPtr hc1,  hc2;

		public:
			MalonicAcid (Molecule_t moltype);
			virtual ~MalonicAcid ();

			static int numMalonicAcid;
			static int numMalonate;
			static int numDimalonate;

			void SetAtoms ();

			AtomPtr CM () const { return cm; }	// methylene carbon
			std::pair<double,double> DihedralAngle ();

	}; // class malonic




	class SuccinicAcid : public Alkane, public Dihedral {
		protected:
			VecR ch2_1, ch2_2;

		public:
			SuccinicAcid ();
			virtual void SetDihedralAtoms();
			VecR Bisector (AtomPtr left, AtomPtr center, AtomPtr right);

			//void SetMethyleneBisectors ();
			//VecR& CH2_1 () { return ch2_1; }
			//VecR& CH2_2 () { return ch2_2; }

	};


	class Formaldehyde : public Alkane {

		public:
			Formaldehyde ();			// a default constructor
			virtual ~Formaldehyde ();
			Formaldehyde (const MolPtr& molecule);
			Formaldehyde (const Molecule& molecule);		// copy constructor for casting from a molecule
			static int numFormaldehyde;

			AtomPtr C () const { return _c; }
			AtomPtr O () const { return _o; }
			AtomPtr H1 () const { return _h1; }
			AtomPtr H2 () const { return _h2; }

			VecR CH1 () const { return _ch1; }
			VecR CH2 () const { return _ch2; }
			VecR CO () const { return _co; }

			void SetAtoms ();
			void SetBonds ();
			VecR ReferencePoint () const { return _c->Position(); }

		protected:
			AtomPtr _c, _o, _h1, _h2;
			VecR _co, _ch1, _ch2;
	};


	class Diacid : public Alkane {
		public:
			Diacid ();			// a default constructor
			virtual ~Diacid ();
			Diacid (const Molecule& molecule);		// copy constructor for casting from a molecule

			// refer to the carbonyl carbons
			AtomPtr CarbonylCarbon1 () { return this->c1; }
			AtomPtr CarbonylCarbon2 () { return this->c2; }

			// bisector of the O-C-O of the carbonyl group
			VecR CarbonylBisector1 ();
			VecR CarbonylBisector2 ();

			// bonds from the carbonyl carbon to the carbonyl oxygens
			VecR CO1 ();
			VecR CO2 ();

			void SetAtoms ();

			// returns the dihedral angle (see implementatin to get it!)
			static std::pair<double,double> MalonicDihedralAngle (Diacid * acid);

			typedef std::list<ThreeAtomGroup> atom_group_list;
			atom_group_list::const_iterator methyls_begin () const { return methyl_groups.begin(); }
			atom_group_list::const_iterator methyls_end () const { return methyl_groups.end(); }

			atom_group_list::const_iterator carbonyls_begin () const { return carbonyl_groups.begin(); }
			atom_group_list::const_iterator carbonyls_end () const { return carbonyl_groups.end(); }

			Atom_ptr_vec methyl_hydrogens () const;
			Atom_ptr_vec carbonyl_hydrogens () const;
			Atom_ptr_vec carbonyl_oxygens () const;

		protected:
			void LoadAtomGroups () { LoadMethylGroups(); LoadCarbonylGroups(); }
			void LoadMethylGroups ();
			void LoadCarbonylGroups ();

			std::list<ThreeAtomGroup> methyl_groups;
			std::list<ThreeAtomGroup> carbonyl_groups;
			std::list<AtomPtr> hydrogens;

			std::pair<AtomPtr,AtomPtr> FindMethylHydrogens (AtomPtr carbon);
	};

	typedef std::vector<Diacid *>::iterator Diacid_it;


}
#endif
