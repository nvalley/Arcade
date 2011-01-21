#ifndef H2O_ANALYSIS_H
#define H2O_ANALYSIS_H

#include "analysis.h"
#include <gsl/gsl_statistics.h>

namespace h2o_analysis {

	using namespace md_analysis;


	// ****************** H2O Manipulator ********************** //

	template <typename T>
		class H2OSystemManipulator : public SystemManipulator<T> {

			public:
				typedef Analyzer<T> system_t;

				H2OSystemManipulator (system_t * t, const int number_of_waters_for_surface_calc = 70) : 
					SystemManipulator<T>(t), 
					reference_point(WaterSystem<T>::SystemParameterLookup("analysis.reference-location")),
					number_surface_waters(number_of_waters_for_surface_calc) { 

						this->_system->LoadWaters();
						// gather all the system waters into an analysis container
						for (Mol_it it = this->_system->int_wats.begin(); it != this->_system->int_wats.end(); it++) {
							WaterPtr wat (new Water(*(*it)));
							wat->SetAtoms();
							all_waters.push_back(wat);
						}

						all_water_atoms.clear();
						std::copy(this->_system->int_atoms.begin(), this->_system->int_atoms.end(), std::back_inserter(all_water_atoms));
						this->Reload();

						top_surface = WaterSystem<T>::SystemParameterLookup("analysis.top-surface");
					}

				virtual ~H2OSystemManipulator () { 
					for (Wat_it it = all_waters.begin(); it != all_waters.end(); it++)
						delete *it;
				}

				void Reload ();
				virtual void FindWaterSurfaceLocation ();		

				double ReferencePoint() const { return reference_point; }
				void ReferencePoint (const double point) { reference_point = point; }
				double SurfaceLocation () const { return surface_location; }
				double SurfaceWidth () const { return surface_width; }	// standard deviation
				//double SurfaceThickness () const { return 

				Wat_it begin() { return analysis_waters.begin(); }
				Wat_it end() { return analysis_waters.end(); }
				Wat_rit rbegin() { return analysis_waters.rbegin(); }
				Wat_rit rend() { return analysis_waters.rend(); }

			protected:
				Water_ptr_vec all_waters, analysis_waters;
				Atom_ptr_vec all_water_atoms;

				double reference_point;	// the original location of the so2 along the reference axis
				int number_surface_waters;
				bool	top_surface;

				double surface_location;	// location of the water surface along the reference axis
				double surface_width;			// standard deviation of the positions of waters used to calculate the surface_location

				// functor to grab a water's location based on the oxygen position
				class WaterLocation : public std::unary_function <WaterPtr, double> {
					public:
						double operator() (const WaterPtr wat) const {
							return system_t::Position(wat);
						}
				}; // water location

		};	// class H2OSystemManipulator


	template <typename T>
		void H2OSystemManipulator<T>::Reload () {

			analysis_waters.clear();
			std::copy (all_waters.begin(), all_waters.end(), std::back_inserter(analysis_waters));
			// now all_waters has... all the waters, and analysis wats is used to perform some analysis
			this->analysis_atoms.clear();
			std::copy (all_water_atoms.begin(), all_water_atoms.end(), std::back_inserter(this->analysis_atoms));
		}	// reload analysis wats


	template <typename T>
		void H2OSystemManipulator<T>::FindWaterSurfaceLocation () {
			// get rid of everything above (or below) the reference point
			if (top_surface) {
				analysis_waters.erase(
						remove_if(analysis_waters.begin(), analysis_waters.end(), system_t::MoleculeAbovePosition(reference_point, system_t::axis)), analysis_waters.end());
			}
			else if (!top_surface) {
				analysis_waters.erase(
						remove_if(analysis_waters.begin(), analysis_waters.end(), system_t::MoleculeBelowPosition(reference_point, system_t::axis)), analysis_waters.end()); // bottom surface
			}

			// sort the waters by position along the reference axis - first waters are lowest, last are highest
			std::sort (analysis_waters.begin(), analysis_waters.end(), system_t::molecule_position_pred(Atom::O));

			// get the position of the top-most waters
			std::vector<double> surface_water_positions;
			if (top_surface) {
				std::transform (analysis_waters.rbegin(), analysis_waters.rbegin()+number_surface_waters, std::back_inserter(surface_water_positions), WaterLocation());
			}	// top surface
			else if (!top_surface) {
				std::transform (analysis_waters.rbegin(), analysis_waters.rbegin()+number_surface_waters, std::back_inserter(surface_water_positions), WaterLocation());
			}	// bottom surface
			surface_location = gsl_stats_mean (&surface_water_positions[0], 1, number_surface_waters);
			surface_width = gsl_stats_sd (&surface_water_positions[0], 1, number_surface_waters);

			if (surface_width > 2.0) {
				std::cout << std::endl << "Check the pbc-flip setting and decrease/increase is to fix this gigantic surface width" << std::endl;
				std::cout << "Here's the positions of the waters used to calculate the surface:" << std::endl;
				std::copy (surface_water_positions.begin(), surface_water_positions.end(), std::ostream_iterator<double>(std::cout, " "));
				printf ("\nSurface width = % 8.3f\n", surface_width);
				fflush(stdout);
			}

		}	// find surface water location

}	// namespace md analysis



#endif
