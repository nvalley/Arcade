#include "malonic-analysis.h"

namespace malonic {

	void COTheta::MoleculeCalculation () {
		if (this->mol->MolType() != Molecule::MALONIC) return;

		this->mol->SetAtoms();

		//this->mol->LoadAtomGroups();

		co_bond = MDSystem::Distance(this->mol->C1(), this->mol->O1());
		theta = acos(co_bond < axis) * 180.0/M_PI;
		angles (theta);

		co_bond = MDSystem::Distance(this->mol->C2(), this->mol->O2());
		theta = acos(co_bond < axis) * 180.0/M_PI;
		angles (theta);
	}




	void CarbonBackboneThetaPhi::MoleculeCalculation () {
		if (this->mol->MolType() != Molecule::MALONIC) return;

		this->mol->SetAtoms();

		//this->mol->Print();

		ccc.SetAtoms(this->mol->C1(), this->mol->CM(), this->mol->C2());

		theta = acos(ccc.Bisector() < axis) * 180.0 / M_PI;

		phi = Dihedral::Angle(axis,ccc.Bisector(),ccc.Bond1()) * 180.0 / M_PI;
		phi = fabs(phi);
		if (phi > 90.0)
			phi = 180.0 - phi;

		//printf ("\ntheta = %f\nphi = %f\n", theta, phi);

		// get the angles binned
		angles (theta, phi);
	}


	void CarboxylicDihedralPsiPsi::MoleculeCalculation () {
		if (mol->MolType() != Molecule::MALONIC) return;
		this->mol->SetAtoms();
		//this->mol->Print();
		ccc.SetAtoms(this->mol->C1(), this->mol->CM(), this->mol->C2());

		std::pair<double,double> psi = this->mol->DihedralAngle();

		angles(fabs(psi.first), fabs(psi.second));

		return;
	}




	RDF::RDF (Analyzer * t) :
		MalonicAnalysis (t,
				std::string ("Malonic RDFs"),
				std::string("")),
		rdf_alc (std::string("MalonicRDF.alcO-H.dat"), 
				0.5, 6.5, 0.05),
		rdf_carb (std::string("MalonicRDF.carbO-H.dat"), 
				0.5, 6.5, 0.05) { }

	void RDF::MoleculeCalculation () {
		this->LoadWaters();
		this->mol->SetAtoms();

		AtomPtr o1 = this->mol->O1();
		AtomPtr o2 = this->mol->O2();
		AtomPtr oh1 = this->mol->OH1();
		AtomPtr oh2 = this->mol->OH2();
		WaterPtr wat;

		for (Mol_it it = this->begin_wats(); it != this->end_wats(); it++) {
			wat = static_cast<WaterPtr>(*it);
			distance = MDSystem::Distance (o1, wat->H1()).norm();
			rdf_carb(distance);
			distance = MDSystem::Distance (o2, wat->H1()).norm();
			rdf_carb(distance);
			distance = MDSystem::Distance (o1, wat->H2()).norm();
			rdf_carb(distance);
			distance = MDSystem::Distance (o2, wat->H2()).norm();
			rdf_carb(distance);

			distance = MDSystem::Distance (oh1, wat->H1()).norm();
			rdf_alc(distance);
			distance = MDSystem::Distance (oh2, wat->H1()).norm();
			rdf_alc(distance);
			distance = MDSystem::Distance (oh1, wat->H2()).norm();
			rdf_alc(distance);
			distance = MDSystem::Distance (oh2, wat->H2()).norm();
			rdf_alc(distance);
		}
	}

	MolecularDipole::MolecularDipole (Analyzer * t) :
		MalonicAnalysis (t,
				std::string ("Malonic molecular dipole"),
				std::string("MalonicDipole.dat")) { }


	void MolecularDipole::MoleculeCalculation () {
		VecR dipole = MDSystem::CalcWannierDipole (this->mol);
		//this->mol->Print();
		//for (vector_map_it wan = this->mol->wanniers_begin(); wan != this->mol->wanniers_end(); wan++) {
		//(*wan).Print();
		//}
		fprintf (this->output, "% 15.8f % 15.8f % 15.8f % 15.8f\n", dipole[x], dipole[y], dipole[z], dipole.norm());
		fflush(this->output);
	}


	BondLengths::BondLengths (Analyzer * t) :
		MalonicAnalysis (t,
				std::string ("Malonic bondlengths"),
				std::string("MalonicBondLengths.dat")) { 
			int numsteps = Analyzer::timesteps;
			lengths[c1o1] = std::vector<double> (numsteps, 0.0);
			lengths[c1oh1] = std::vector<double> (numsteps, 0.0);
			lengths[c2o2] = std::vector<double> (numsteps, 0.0);
			lengths[c2oh2] = std::vector<double> (numsteps, 0.0);
			lengths[h1o2] = std::vector<double> (numsteps, 0.0);
			lengths[h2o1] = std::vector<double> (numsteps, 0.0);
			lengths[h1oh1] = std::vector<double> (numsteps, 0.0);
			lengths[h2oh2] = std::vector<double> (numsteps, 0.0);
			lengths[h1oh2] = std::vector<double> (numsteps, 0.0);
			lengths[h2oh1] = std::vector<double> (numsteps, 0.0);
			lengths[o1waterh] = std::vector<double> (numsteps, 0.0);
			lengths[o2waterh] = std::vector<double> (numsteps, 0.0);
		}


	void BondLengths::CalcDistance (AtomPtr atom1, AtomPtr atom2, bond_t bond) {
		lengths[bond].operator[](Analyzer::timestep) = MDSystem::Distance(atom1, atom2).norm();
	}

	void BondLengths::MoleculeCalculation () {
		if (this->mol->MolType() != Molecule::MALONIC) return;
		this->mol->SetAtoms();

		CalcDistance(this->mol->C1(), this->mol->O1(), c1o1);
		CalcDistance(this->mol->C2(), this->mol->O2(), c2o2);
		CalcDistance(this->mol->C1(), this->mol->OH1(), c1oh1);
		CalcDistance(this->mol->C2(), this->mol->OH2(), c2oh2);

		//if (this->mol->H1() != (AtomPtr)NULL) {
		CalcDistance(this->mol->H1(), this->mol->OH1(), h1oh1);
		CalcDistance(this->mol->H1(), this->mol->OH2(), h1oh2);
		CalcDistance(this->mol->H1(), this->mol->O2(), h1o2);
		//}

		//if (this->mol->H2() != (AtomPtr)NULL) {
		CalcDistance(this->mol->H2(), this->mol->OH2(), h2oh2);
		CalcDistance(this->mol->H2(), this->mol->OH1(), h2oh1);
		CalcDistance(this->mol->H2(), this->mol->O1(), h2o1);
		//}

		// now find the water H that's closest to the carbonyl oxygens
		// sort the waters by distance to each oxygen
		/*
			 this->LoadWaters();
			 std::sort(this->begin_wats(), this->end_wats(), Analyzer::molecule_distance_cmp(this->mol->O1()));
			 (*this->begin_wats())->SetAtoms();
			 CalcDistance(this->mol->O1(), (*this->begin_wats())->GetAtom("O"), o1waterh);

			 std::sort(this->begin_wats(), this->end_wats(), Analyzer::molecule_distance_cmp(this->mol->O2()));
			 (*this->begin_wats())->SetAtoms();
			 CalcDistance(this->mol->O2(), (*this->begin_wats())->GetAtom("O"), o2waterh);
			 */
	}

	void BondLengths::OutputDataPoint (bond_t bond, int timestep) {
		fprintf (this->output, " %6.4f", lengths[bond].operator[](timestep));
	}

	void BondLengths::DataOutput () {
		rewind (this->output);

		fprintf (this->output, "c1o1 c2o2 c1oh1 c2oh2 h1oh1 h1oh2 h1o2 h2oh2 h2oh1 h2o1\n");
		for (int i = 0; i < Analyzer::timesteps; i++) {
			OutputDataPoint (c1o1, i);
			OutputDataPoint (c2o2, i);
			OutputDataPoint (c1oh1, i);
			OutputDataPoint (c2oh2, i);
			OutputDataPoint (h1oh1, i);
			OutputDataPoint (h1oh2, i);
			OutputDataPoint (h1o2, i);
			OutputDataPoint (h2oh2, i);
			OutputDataPoint (h2oh1, i);
			OutputDataPoint (h2o1, i);
			//OutputDataPoint (o1waterh, i);
			//OutputDataPoint (o2waterh, i);
			fprintf (this->output, "\n");
		}
	}




	void MalonicTest::MoleculeCalculation () {
		//this->mol->SetAtoms();

		AtomPtr h;
		AtomPtr o;
		double distance;

		h = this->mol->H1();
		if (h != NULL) {
			o = this->mol->O2();
			distance = MDSystem::Distance(o,h).norm();
			printf ("distance = %f\n", distance);
		}

		h = this->mol->H2();
		if (h != NULL) {
			o = this->mol->O1();
			distance = MDSystem::Distance(o,h).norm();
			printf ("distance = %f\n", distance);
		}
	}



	/*
	// for each timestep we're just finding the distances between the given pairs of atoms and outputing them all
	// to a data file.
	void MalonicBondLengthAnalysis::Analysis () {
	this->LoadAll();

	for (std::vector< std::pair<int,int> >::iterator it = atom_ids.begin(); it != atom_ids.end(); it++) {
	fprintf (this->output, "%13.5f ", BondLength(it->first,it->second));
	}
	fprintf (this->output, "\n");
	fflush(this->output);
	}	

	double MalonicBondLengthAnalysis::BondLength (const int id1, const int id2) {

	atom1 = this->Atoms()[id1];
	atom2 = this->Atoms()[id2];

	distance = MDSystem::Distance(atom1,atom2).Magnitude();

	return distance;
	}


	void MethyleneTilt::Analysis () {
	this->LoadAll();

	for (Mol_it mol = this->begin_mols(); mol != this->end_mols(); mol++) {
	if ((*mol)->MolType() != Molecule::MALONIC 
	&& (*mol)->MolType() != Molecule::MALONATE 
	&& (*mol)->MolType() != Molecule::DIMALONATE) continue;

	mal = static_cast<alkane::MalonicAcid *>(*mol);

// methylene carbon is index 3
// methylene hydrogen IDs are 7 and 10
// However, that corresponds to atom #:
// carbon = 3, hyd1 = 6, hyd2 = 7
bisector = Molecule::Bisector(mal->GetAtom(6), mal->GetAtom(3), mal->GetAtom(7));
angle = bisector < VecR::UnitZ();
angle = acos(angle)*180.0/M_PI;

histo (angle);

}

}
*/


}
