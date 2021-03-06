#ifndef ANGLE_ANALYSIS_H
#define ANGLE_ANALYSIS_H

#include "analysis.h"
#include "manipulators.h"
#include "histogram-analysis.h"
#include "neighbor-analysis.h"
#include "molecule-analysis.h"
#include "h2o-analysis.h"


namespace angle_analysis {

	using namespace md_analysis;

	class AngleHelper {
		public:
			typedef Analyzer system_t;

			AngleHelper (system_t * t,
					double min1, double max1, double res1,
					double min2, double max2, double res2,
					std::string alphafile = std::string("alpha.dat"), 
					std::string betafile = std::string("beta.dat"))
				:
					_system(t), 
					_alpha(alphafile, min1, max1, res1, min2, max2, res2),
					_beta(betafile, min1, max1, res1, min2, max2, res2) {
						double r[9] = {1,0,0,0,0,1,0,-1,0};
						_rotation = MatR(r);
					}


			virtual ~AngleHelper () { }

			void Alpha (const double val1, const double val2) { _alpha (val1, val2); }
			void Beta (const double val1, const double val2) { _beta (val1, val2); }

			double Alpha_TotalCount() const { return _alpha.TotalCount(); }
			double Beta_TotalCount() const { return _beta.TotalCount(); }

			virtual void DataOutput() {
				_alpha.OutputData();
				_beta.OutputData();
			}

		protected:

			system_t *	_system;

			// histograms are indexed by [position, angle]
			Histogram2DAgent		_alpha;
			Histogram2DAgent		_beta;

			MatR _rotation;		// rotates a vector from the system x-y-z axes to the analysis frame where
			MatR _dcm;

	};	 // angle helper



	class DistanceAngleHelper : public AngleHelper {
		public:
			typedef Analyzer system_t;

			DistanceAngleHelper (system_t * t)
				:	AngleHelper(t, 
						WaterSystem::posmin, WaterSystem::posmax, system_t::posres, 
						system_t::angmin, system_t::angmax, system_t::angres) { }

			virtual ~DistanceAngleHelper () { }
	};

	class AngleAngleHelper : public AngleHelper {
		public:
			typedef Analyzer system_t;

			AngleAngleHelper (system_t * t)
				:	AngleHelper(t, 
						system_t::angmin, system_t::angmax, system_t::angres,
						system_t::angmin, system_t::angmax, system_t::angres) { }

			virtual ~AngleAngleHelper () { }
	};





	// functor takes a water and returns the values of the cos(angles) formed between the two oh-vectors. The first value of the pair is always the greater (magnitude) of the two values.
	class OHAngleCalculator : public std::unary_function <WaterPtr,std::pair<double,double> > {
		private:
			VecR axis;	// the reference axis to which the angles will be formed

		public:
			typedef std::pair<double,double> angle_pair_t;
			OHAngleCalculator (const VecR ax) : axis(ax) { }
			angle_pair_t operator() (const WaterPtr& wat);
	};


	/************** H2O Angle Analysis **********************/
	/* An analysis to determine the spatial and angular distribution of waters in a slab system.
	 * Each water has an internal coordinate frame such that the molecular bisector points in the positive Z direction, and the molecular plane is in the molecular x-z plane.
	 * Additionally, a water's orientation is described by two angles - alpha & beta.
	 * Theta:
	 *		The angle formed between the water's molecular bisector and the system's positive Z-axis.
	 *		Theta ranges from 0 to 180 degrees, but is reported as cos(alpha) values ranging from +1 to -1
	 * Phi:
	 *		The angle of twist around the system Z-axis (referenced with the system X-axis as the 0 degree)
	 *		The angle is that one formed by the projection of the water's bisector onto the system's x-y plane and the positive system x axis.
	 *		Phi ranges from -180 to +180 degrees, but is reported in radians.
	 *
	 */

	class H2OAngleAnalysis : public AnalysisSet {
		public:
			typedef Analyzer system_t;

			H2OAngleAnalysis (system_t * t) :
				AnalysisSet(t,
						std::string ("H2O Angle Analysis"),
						std::string ("")),
				h2os(t),
				angles(t) { 
					h2os.ReferencePoint(WaterSystem::SystemParameterLookup("analysis.reference-location"));
				}

			virtual void BinAngles (MolPtr mol);
			virtual void Analysis ();
			void DataOutput () { angles.DataOutput(); }

		protected:
			h2o_analysis::H2OSystemManipulator	h2os;
			DistanceAngleHelper								angles;
	};


	class OHAngleAnalysis : public AnalysisSet {
		public:
			typedef Analyzer system_t;

			OHAngleAnalysis (system_t * t) :
				AnalysisSet(t,
						std::string ("Water OH Angle Analysis"),
						std::string ("")),
				h2os(t),
				_alpha("oh-angles.both.dat", 
						-20.0,20.0,0.5,
						-1.0, 1.0, 0.02),
				oh_calculator(VecR::UnitY()) { 
					h2os.ReferencePoint(WaterSystem::SystemParameterLookup("analysis.reference-location"));
				}

			virtual void Analysis ();
			void DataOutput () { _alpha.OutputData(); }

		protected:
			h2o_analysis::H2OSystemManipulator	h2os;
			md_analysis::Histogram2DAgent				_alpha;
			OHAngleCalculator										oh_calculator;
			double distance;
	};	// water oh angle analysis




	// *********** water OH angle analysis ****************
	class WaterOHAngleAnalysis : public AnalysisSet {
		public:
			typedef Analyzer system_t;

			WaterOHAngleAnalysis (system_t * t) :
				AnalysisSet(t,
						std::string ("Water OH Angle Analysis - via SO2 transit"),
						std::string ("")),
				h2os(t),
				_alpha("alpha.dat", 
						WaterSystem::posmin, WaterSystem::posmax, system_t::posres,
						system_t::angmin, system_t::angmax, system_t::angres),
				oh_calculator(VecR::UnitY()) { 
					h2os.ReferencePoint(WaterSystem::SystemParameterLookup("analysis.reference-location"));
				}

			~WaterOHAngleAnalysis () { }
			virtual void Analysis ();
			void DataOutput () { _alpha.OutputData(); }

		protected:
			h2o_analysis::H2OSystemManipulator	h2os;
			md_analysis::Histogram2DAgent				_alpha;
			OHAngleCalculator										oh_calculator;
			double distance;
	};	// water oh angle analysis




	// functor takes a water and returns the values of the cos(angles) formed between the two oh-vectors. The first value of the pair is always the greater (magnitude) of the two values.
	class SOAngleCalculator : public std::unary_function <SulfurDioxide*,std::pair<double,double> > {
		private:
			VecR axis;	// the reference axis to which the angles will be formed
		public:
			SOAngleCalculator (const VecR ax) : axis(ax) { }
			std::pair<double,double> operator() (const SulfurDioxide* so2);
	};


	/************** Angle analysis of the reference SO2 **************/

	class ReferenceSO2AngleAnalysis : public AnalysisSet {
		protected:
			h2o_analysis::H2OSystemManipulator	h2os;
			so2_analysis::SO2SystemManipulator	so2s;
			DistanceAngleHelper								angles;

		public:

			typedef Analyzer system_t;

			ReferenceSO2AngleAnalysis (system_t * t, 
					std::string description = std::string("Angle analysis of the reference SO2"), 
					std::string fn = std::string("")) :
				AnalysisSet (t, description, fn),
				h2os(t), so2s(t), angles(t) { 
					h2os.ReferencePoint(WaterSystem::SystemParameterLookup("analysis.reference-location"));
				}

			virtual ~ReferenceSO2AngleAnalysis () { } 

			virtual void Analysis ();
			virtual void DataOutput () { angles.DataOutput(); }
			virtual void BinAngles (SulfurDioxide * so2);

	};	// reference so2 angle analysis





	/************** SO-bond angle analysis **************/
	class SOAngleAnalysis : public AnalysisSet {
		public:
			typedef Analyzer system_t;

			SOAngleAnalysis (system_t * t) :
				AnalysisSet(t,
						std::string ("SO2 SO Angle Analysis"),
						std::string ("")),
				h2os(t),
				so2s(t),
				_alpha("alpha.dat", 
						WaterSystem::posmin, WaterSystem::posmax, system_t::posres,
						system_t::angmin, system_t::angmax, system_t::angres),
				so_calculator(VecR::UnitY()) { 
					h2os.ReferencePoint(WaterSystem::SystemParameterLookup("analysis.reference-location"));
				}

			~SOAngleAnalysis () { }
			virtual void Analysis ();
			void DataOutput () { _alpha.OutputData(); }

		protected:
			h2o_analysis::H2OSystemManipulator	h2os;
			so2_analysis::SO2SystemManipulator	so2s;
			md_analysis::Histogram2DAgent										_alpha;
			SOAngleCalculator									so_calculator;
			double distance;
	};	// so2 angle analysis




	/************ so2 transit first-binding water angle analysis ************/

	class SO2AdsorptionWaterAngleAnalysis : public AnalysisSet {
		public:
			typedef Analyzer system_t;

			SO2AdsorptionWaterAngleAnalysis(system_t * t) 
				: 
					AnalysisSet (t,
							std::string("Analysis of waters near an adsorbing so2"),
							std::string("first-adsorption-water.dat")),
					h2os(t), so2s(t), nm(t), first_bound_water ((WaterPtr)NULL), second_pass(false)
					//angle_histo ("angle.dat", 0.0, 10.0, 0.1, -1.0, 1.0, 0.05)	// distance from 0 to 10 angstroms
		{ }
			~SO2AdsorptionWaterAngleAnalysis () {
				if (!first_bound_water) delete first_bound_water;
			}

			virtual void Analysis ();
			//virtual void DataOutput () { angle_histo.OutputData(); }

			void FindInteractions ();

		protected:
			h2o_analysis::H2OSystemManipulator	h2os;
			so2_analysis::SO2SystemManipulator	so2s;
			neighbor_analysis::NeighborManipulator	nm;

			bondgraph::BondGraph	graph;
			std::vector<double>		so2_distances;
			std::vector<double>		water_angles;
			WaterPtr							first_bound_water;
			Atom_ptr_vec					bonded_atoms;
			Atom_ptr_vec					analysis_atoms;
			bool									second_pass;

			// predicate for checking atom residue types
			static bool residue_eq (const AtomPtr a, const std::string& res);
	};


	class WaterOrientationNearSO2 : public AnalysisSet {

		public:
			typedef Analyzer system_t;
			WaterOrientationNearSO2 (system_t * t) 
				: 
					AnalysisSet (t,
							std::string("Angle analysis of waters relative to so2 distance"),
							std::string("")),

					angles (t, 
							1.4, 15.0, 0.05,
							//												-1.0, 1.0, 0.05) { }
					0.0, 180.0, 1.0) { }

			void Analysis ();
			void DataOutput () { angles.DataOutput(); }

		private:
			AngleHelper	angles;	// 2d histogram

	};	// water orientation near so2


	class PsiPsiAgent {
		protected:
			double theta1, theta2;
			Multi2DHistogramAgent	histos;
			VecR axis, v1;

		public:
			PsiPsiAgent (
					std::string prefix,
					std::string suffix,
					float a, float b, float c, 
					float d, float e, float f, 
					float g, float h, float i) :
				axis(VecR::UnitY()),
				histos (
						a,b,c,
						d,e,f,
						g,h,i,
						prefix, suffix) { }

			void operator() 
				( VecR v1, VecR v2, 
					h2o_analysis::surface_distance_t position);

			void Override (double position, double v1, double v2) {
				histos (position, v1, v2);
			}

			void DataOutput () {
				//DivideByBothSineDegrees func;
				DoNothing2D func;
				histos.DataOutput(func);
			}
	};


	class ThetaPhiAgent {
		protected:
			double theta, phi;
			Multi2DHistogramAgent	histos;
			VecR axis, v1;

		public:
			ThetaPhiAgent (
					std::string prefix,
					std::string suffix,
					float a, float b, float c, 
					float d, float e, float f, 
					float g, float h, float i) :
				axis(VecR::UnitY()),
				histos (
						a,b,c,
						d,e,f,
						g,h,i,
						prefix, suffix) { }

			void operator() 
				( VecR bisector, VecR ref_bond, 
					h2o_analysis::surface_distance_t position);

			void Override (double position, double v1, double v2) {
				histos (position, v1, v2);
			}

			void DataOutput () {
				DivideByLeftSineDegrees func;
				histos.DataOutput(func);
			}

			void DataOutputBothDivided () {
				DivideByBothSineDegrees func;
				histos.DataOutput(func);
			}
	};

	class PositionThetaAgent {
		protected:
			Histogram2DAgent histo;

		public:
			PositionThetaAgent (std::string filename,
					double min1, double max1, double res1, 
					double min2, double max2, double res2)
				:	
					histo (filename, min1, max1, res1, min2, max2, res2) { }

			void operator() (const double position, const double theta) {
				histo(position, theta);
			}

			void OutputData () {
				histo.OutputData ();
			}
			void OutputData (DataOutput2DFunction& func) {
				histo.OutputData (func);
			}
	};



}	// namespace angle analysis

#endif
