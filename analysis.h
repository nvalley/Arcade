#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include "watersystem.h"
#include "utility.h"
#include "patterns.h"
#include "dataoutput.h"


namespace md_analysis {

	using namespace md_system;
	using namespace md_files;

	template <class T> class Analyzer;

	// An analysis that will be performed on a system by an analyzer
	template <typename T>
		class AnalysisSet {
			public:
				typedef Analyzer<T> system_t;

				virtual ~AnalysisSet () {
					if (output != (FILE *)NULL)
						fclose(output);
				}

				AnalysisSet (system_t * sys, std::string desc, std::string fn) 
					: 
						_system(sys),
						description (desc), filename(fn),
						output((FILE *)NULL) { }

				// default setup
				virtual void Setup () {
					OpenDataOutputFile ();
					_system->LoadAll();
					return;
				}

				// each analyzer has to have an analysis function to do some number crunching
				virtual void Analysis () = 0;
				// normally this can be done in the analysis section, but just for style we can have something different defined here
				virtual void DataOutput () { }
				virtual void PostAnalysis () { }

				void OpenDataOutputFile ();

				std::string& Description () { return description; }
				std::string& Filename () { return filename; }

				void LoadAll () const { this->_system->LoadAll(); }
				void LoadWaters () const { this->_system->LoadWaters(); }
				Atom_it_non_const begin () const { return this->_system->sys_atoms.begin(); }
				Atom_it_non_const end () const { return this->_system->sys_atoms.end(); }

				Mol_it begin_mols () const { return this->_system->sys_mols.begin(); }
				Mol_it end_mols () const { return this->_system->sys_mols.end(); }

				Mol_it begin_wats () const { return this->_system->int_wats.begin(); }
				Mol_it end_wats () const { return this->_system->int_wats.end(); }

				wannier_it begin_wanniers () const { return this->_system->begin_wanniers(); }
				wannier_it end_wanniers () const { return this->_system->end_wanniers(); }


			protected:

				system_t * _system;
				std::string description;	// describes the analysis that is performed
				std::string filename;		// filename to use for data output
				FILE * output;


		};  // class AnalysisSet


	template <class T> 
		void AnalysisSet<T>::OpenDataOutputFile () {

			output = (FILE *)NULL;
			if (filename == "") {
				printf ("\nAnalysis:: No filename specified for dataoutput.\n");
			}
			else {
				output = fopen(filename.c_str(), "w");

				if (output == (FILE *)NULL) {
					printf ("AnalysisSet<T>::_OpenDataOutputFile() - couldn't open the data output file, \"%s\", given in the analysis set!\n", filename.c_str());
					exit(1);
				}

				printf ("\nOutputting data to \"%s\"\n", filename.c_str());
			}
			return;
		}


	// manipulator superclass
	template <typename T>
		class SystemManipulator {
			public:
				typedef Analyzer<T> system_t;

				SystemManipulator (system_t * sys) : _system(sys) { 
					_system->LoadAll();
					std::copy(_system->sys_atoms.begin(), _system->sys_atoms.end(), std::back_inserter(all_atoms));
					std::copy(_system->sys_mols.begin(), _system->sys_mols.end(), std::back_inserter(all_mols));
					Reload();
				}

				virtual ~SystemManipulator () { }

				// reload all the analysis atoms and molecules
				virtual void Reload () {
					analysis_atoms.clear();
					analysis_mols.clear();
					std::copy(all_atoms.begin(), all_atoms.end(), std::back_inserter(analysis_atoms));
					std::copy(all_mols.begin(), all_mols.end(), std::back_inserter(analysis_mols));
				}

				Atom_it atoms_begin() { return analysis_atoms.begin(); }
				Atom_it atoms_end() { return analysis_atoms.end(); }

				Atom_it mols_begin() { return analysis_mols.begin(); }
				Atom_it mols_end() { return analysis_mols.end(); }

			protected:
				system_t * _system;

				Atom_ptr_vec		all_atoms;
				Mol_ptr_vec			all_mols;

				Atom_ptr_vec		analysis_atoms;
				Mol_ptr_vec			analysis_mols;
		};	// system manipulator




	template <class T>
		class Analyzer : public WaterSystem<T>, public patterns::observer::observable {

			protected:

				int	output_freq;

				void _OutputHeader () const;
				void _OutputStatus ();
				//md_analysis::StarStatusBarUpdater	status_updater;
				md_analysis::PercentProgressBar	status_updater;

			public:
				Analyzer (const std::string = std::string("system.cfg"));
				virtual ~Analyzer ();

				typedef AnalysisSet<T> analysis_t;
				void SystemAnalysis (analysis_t&);

				// position boundaries and bin widths for gathering histogram data
				static double	posres;
				static int		posbins;
				static double angmin, angmax, angres;
				static int		angbins;
				int						timestep;
				int						Timestep () const { return timestep; }
				static int 		timesteps;
				static int		restart;

				static double Position (const MolPtr);
				static double Position (const AtomPtr);
				static double Position (const VecR&);
				static double Position (const double);

				void LoadNext ();
				void Rewind() { 
					this->sys->Rewind();
					timestep = 1;
				}


				Atom_ptr_vec& Atoms () { return WaterSystem<T>::int_atoms; } 
				Mol_ptr_vec& Molecules () { return WaterSystem<T>::int_mols; }
				Water_ptr_vec& Waters () { return WaterSystem<T>::int_wats; }

				// calculate the system's center of mass
				template <typename Iter> static VecR CenterOfMass (Iter first, Iter last);

				//! Predicate for sorting a container of molecules based on position along the main axis of the system, and using a specific element type to determine molecular position. i.e. sort a container of waters based on the O position, or sort a container of NO3s based on the N position, etc.
				class molecule_position_pred; 		
				class atomic_distance_cmp;
				class molecule_distance_cmp;
				class molecule_distance_generator;

				class MoleculeAbovePosition;
				class MoleculeBelowPosition;

		};	// Analyzer


	template <class T> double	Analyzer<T>::posres;
	template <class T> int		Analyzer<T>::posbins;

	template <class T> double	Analyzer<T>::angmin;
	template <class T> double	Analyzer<T>::angmax;
	template <class T> double	Analyzer<T>::angres;
	template <class T> int		Analyzer<T>::angbins;

	template <class T> int 		Analyzer<T>::timesteps;
	template <class T> int		Analyzer<T>::restart;

	template <class T> 
		Analyzer<T>::Analyzer (const std::string ConfigurationFilename) : 
			WaterSystem<T>(ConfigurationFilename),
			output_freq(WaterSystem<T>::SystemParameterLookup("analysis.output-frequency")),
			timestep (0)
	{ 
		Analyzer<T>::posres = WaterSystem<T>::SystemParameterLookup("analysis.position-range")[2];
		Analyzer<T>::posbins = int((WaterSystem<T>::posmax - WaterSystem<T>::posmin)/posres);

		Analyzer<T>::angmin = WaterSystem<T>::SystemParameterLookup("analysis.angle-range")[0];
		Analyzer<T>::angmax = WaterSystem<T>::SystemParameterLookup("analysis.angle-range")[1];
		Analyzer<T>::angres = WaterSystem<T>::SystemParameterLookup("analysis.angle-range")[2];
		Analyzer<T>::angbins = int((angmax - angmin)/angres);

		Analyzer<T>::timesteps = WaterSystem<T>::SystemParameterLookup("system.timesteps");
		Analyzer<T>::restart = WaterSystem<T>::SystemParameterLookup("analysis.restart-time");

		status_updater.Set (output_freq, timesteps, 0);
		this->registerObserver(&status_updater);

		this->_OutputHeader();
	} // Analyzer ctor



	template <class T> 
		void Analyzer<T>::_OutputHeader () const {

			printf ("Analysis Parameters:\n\tScreen output frequency = 1/%d\n\n\tPosition extents for analysis:\n\t\tMin = % 8.3f\n\t\tMax = % 8.3f\n\t\tPosition Resolution = % 8.3f\n\n\tPrimary Axis = %d\nNumber of timesteps to be analyzed = %d\n",
					output_freq, Analyzer<T>::posmin, Analyzer<T>::posmax, Analyzer<T>::posres, int(Analyzer<T>::axis), Analyzer<T>::timesteps);

#ifdef AVG
			printf ("\n\nThe analysis is averaging about the two interfaces located as:\n\tLow  = % 8.3f\n\tHigh = % 8.3f\n\n", int_low, int_high);
#endif
			return;
		}

	template <class T> 
		Analyzer<T>::~Analyzer () {

			delete this->sys;
			return;
		}

	template <class T> 
		void Analyzer<T>::_OutputStatus ()
		{
			this->notifyObservers ();
			return;
		}


	template <typename T>
		void Analyzer<T>::LoadNext () {
			this->sys->LoadNext();
			return;
		}

	//template <>
		//extern void Analyzer<AmberSystem>::LoadNext () {
			//this->sys->LoadNext();
			//return;
		//}

	/*
		 template <>
		 extern void Analyzer<gromacs::GMXSystem<gromacs::TRRFile> >::LoadNext () {
		 this->sys->LoadNext();
		 return;
		 }

		 template <>
		 extern void Analyzer<gromacs::GMXSystem<gromacs::XTCFile> >::LoadNext () {
		 this->sys->LoadNext();
		 return;
		 }
		 */

	template <class T>
		void Analyzer<T>::SystemAnalysis (analysis_t& an) {
			// do some initial setup
			an.Setup();

			// start the analysis - run through each timestep
			for (timestep = 0; timestep < timesteps; timestep++) {

				try {
					// Perform the main loop analysis that works on every timestep of the simulation
					an.Analysis ();
				} catch (std::exception& ex) {
					std::cout << "Caught an exception during the system analysis at timestep " << timestep << "." << std::endl;
					throw;
				}

				// output the status of the analysis (to the screen or somewhere useful)
				this->_OutputStatus ();
				// Output the actual data being collected to a file or something for processing later
				if (!(timestep % (output_freq * 10)) && timestep)
					an.DataOutput();

				try {
					// load the next timestep
					this->LoadNext();
				} catch (std::exception& ex) {
					throw;
				}
			}

			// do one final data output to push out the finalized data set
			an.DataOutput();

			// do a little work after the main analysis loop (normalization of a histogram? etc.)
			an.PostAnalysis ();
			return;
		} // System Analysis w/ analysis set



	/* Find the periodic-boundary-satistfying location of a molecule, atom, vector, or raw coordinate along the reference axis */
	template <class T> 
		double Analyzer<T>::Position (const MolPtr mol) {
			return Analyzer<T>::Position(mol->ReferencePoint());
		}

	template <class T> 
		double Analyzer<T>::Position (const AtomPtr patom) {
			//return Analyzer<T>::Position(patom->Position());
			return WaterSystem<T>::AxisPosition(patom);
		}

	template <class T> 
		double Analyzer<T>::Position (const VecR& v) {
			double position = v[WaterSystem<T>::axis];
			return Analyzer<T>::Position(position);
		}

	template <class T> 
		double Analyzer<T>::Position (const double d) {
			double pos = d;
			if (pos < WaterSystem<T>::pbcflip) pos += MDSystem::Dimensions()[WaterSystem<T>::axis];
			return pos;
		}


	template <class T>
		template <class Iter>	// Has to be iterators to a container of molecules
		VecR Analyzer<T>::CenterOfMass (Iter first, Iter last)
		{
			double mass = 0.0;
			VecR com;
			com.setZero();

			typedef typename std::iterator_traits<Iter>::value_type val_t;

			for (Iter it = first; it != last; it++) {
				for (Atom_it jt = (*it)->begin(); jt != (*it)->end(); jt++) {
					mass += (*jt)->Mass();
					com += (*jt)->Position() * (*jt)->Mass();
				}
			}
			com /= mass;
			return com;
		}

	//! Predicate for sorting molecules based on their positions along the system reference axis. The position of the element supplied (elmt) is used. e.g. if elmt = Atom::O, then the first oxygen of the molecule will be used
	template<class T>
		class Analyzer<T>::molecule_position_pred : public std::binary_function <Molecule*,Molecule*,bool> {
			private:
				Atom::Element_t _elmt;	// determines the element in a molecule to use for position comparison
			public:
				//! upon instantiation, the element to be used for specifying molecular position is provided
				molecule_position_pred (const Atom::Element_t elmt) : _elmt(elmt) { }

				bool operator()(const Molecule* left, const Molecule* right) const {
					AtomPtr left_o = left->GetAtom(_elmt);
					AtomPtr right_o = right->GetAtom(_elmt);
					double left_pos = Analyzer<T>::Position(left_o);
					double right_pos = Analyzer<T>::Position(right_o);

					return left_pos < right_pos;
				}
		};



	// this predicate is used for distance calculations/sorting between atoms given a reference atom or position
	template<class T>
		class Analyzer<T>::atomic_distance_cmp : public std::binary_function <AtomPtr,AtomPtr,bool> {
			private:
				VecR _v;	// the molecule that will act as the reference point for the comparison
			public:
				atomic_distance_cmp (const AtomPtr refatom) : _v (refatom->Position()) { }
				atomic_distance_cmp (const VecR v) : _v (v) { }
				// return the distance between the two molecules and the reference mol
				bool operator()(const AtomPtr left, const AtomPtr right) const {
					double left_dist = MDSystem::Distance(left->Position(), _v).norm();
					double right_dist = MDSystem::Distance(right->Position(), _v).norm();
					return left_dist < right_dist;
				}
		};

	// given a reference point, this returns a molecule's distance to that point
	template <class T>
		class Analyzer<T>::molecule_distance_generator : public std::unary_function <MolPtr,double> {
			private:
				VecR _v;	// the molecule that will act as the reference point for the comparison

			public:
				molecule_distance_generator (const MolPtr refmol) : _v(refmol->ReferencePoint()) { }
				molecule_distance_generator (const AtomPtr refatom) : _v(refatom->Position()) { }
				molecule_distance_generator (const VecR v) : _v(v) { }

				double operator()(const MolPtr mol) const {
					return MDSystem::Distance(mol->ReferencePoint(), _v).norm();
				}
		}; // molecule distance generator

	template<class T>
		class Analyzer<T>::molecule_distance_cmp : public std::binary_function <MolPtr,MolPtr,bool> {
			private:
				VecR _v;	// the molecule that will act as the reference point for the comparison
			public:
				molecule_distance_cmp (const MolPtr refmol) : _v(refmol->ReferencePoint()) { }
				molecule_distance_cmp (const AtomPtr refatom) : _v(refatom->Position()) { }
				molecule_distance_cmp (const VecR v) : _v(v) { }

				// return the distance between the two molecules and the reference
				bool operator()(const MolPtr left, const MolPtr right) const {
					/*
					left->Print();
					left->SetAtoms();
					left->ReferencePoint().Print();
					_v.Print();
					*/
					double left_dist = MDSystem::Distance(left->ReferencePoint(), _v).norm();
					double right_dist = MDSystem::Distance(right->ReferencePoint(), _v).norm();
					return left_dist < right_dist;
				}
		};

	// predicate tells if a molecule's reference point along a given axis is above a given value
	template <typename T>
		class Analyzer<T>::MoleculeAbovePosition : public std::unary_function <MolPtr,bool> {
			private:
				double position;
				coord axis;
			public:
				MoleculeAbovePosition (const double pos, const coord ax) : position(pos), axis(ax) { }
				bool operator() (const MolPtr mol) {
					return Analyzer<T>::Position(mol->ReferencePoint()) > position;
				}
		};

	// predicate tells if a molecule's reference point along a given axis is above a given value
	template <typename T>
		class Analyzer<T>::MoleculeBelowPosition : public std::unary_function <MolPtr,bool> {
			private:
				double position;
				coord axis;
			public:
				MoleculeBelowPosition (const double pos, const coord ax) : position(pos), axis(ax) { }
				bool operator() (const MolPtr mol) {
					return Analyzer<T>::Position(mol->ReferencePoint()) < position;
				}
		};





	/***************** Analysis Sets specific to given MD systems ***************/
	/*
		 class XYZAnalysisSet : public AnalysisSet< Analyzer<XYZSystem> > { 
		 public:
		 typedef Analyzer<XYZSystem> system_t;
		 XYZAnalysisSet (std::string desc, std::string fn) :
		 AnalysisSet<system_t> (desc, fn) { }
		 virtual ~XYZAnalysisSet () { }
		 };

		 class AmberAnalysisSet : public AnalysisSet< Analyzer<AmberSystem> > { 
		 public:
		 typedef Analyzer<AmberSystem> system_t;
		 AmberAnalysisSet (std::string desc, std::string fn) :
		 AnalysisSet< system_t > (desc, fn) { }
		 virtual ~AmberAnalysisSet () { }
		 };
		 */

}
#endif
