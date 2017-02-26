//
// Created by alex on 24.02.17.
//

#include <hj_algo.h>
#include "rapidjson/document.h"
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <GL/glew.h>


pmols::HJPacker::HJPacker(HJParams params) {
    this->params = params;
    init();
}

void pmols::HJPacker::init() {
    Molecule mol(params.mol_file);
    std::cout << "mol red: " << mol.AtomsCount() << std::endl;
    std::tuple<float, float, float> shellSize;
    
    float shell_length, shell_width, shell_height;
    int atoms_count_x, atoms_count_y, atoms_count_z;
    glm::vec3 appos_point;
    
    std::tie(appos_point, shellSize) = mol.GetRectangularShell();
    std::tie(shell_length, shell_width, shell_height) = shellSize;

    float exp_length = params.box_length * params.expansivity;
    float exp_width = params.box_width * params.expansivity;
    float exp_height = params.box_height * params.expansivity;
    
    atoms_count_x = (int)ceilf(exp_length / shell_length);
    atoms_count_y = (int)ceilf(exp_width / shell_width);
    atoms_count_z = (int)ceilf(exp_height / shell_height);

    float max_radius = 0;
    float radius;

    std::cout << "atoms_count: " << atoms_count_x << " " << atoms_count_y << " " << atoms_count_z << std::endl;

    for (int i = 0; i < mol.AtomsCount(); ++i) {
        radius = mol.GetAtom(i).vdw_radius;
        if (radius > max_radius)
            max_radius = radius;
    }

    std::cout << "max atom radius: " << max_radius << std::endl;


    cellLinkedLists = std::make_shared<CellLinkedLists>(2.5 * max_radius, [](Atom *a, Atom *b) -> float {
        //float dist = std::fabs((float)glm::distance(a->coord, b->coord) - a->vdw_radius - b->vdw_radius);
        const float EPS_4 = /*38.5472f*/ 1.f;
//        const float SIGMA = 3.1506f;
        const float SIGMA = /*2.352f*/ 2.06;

        float dist = (float)glm::distance(a->coord, b->coord);
        float energy = EPS_4 * (std::pow(SIGMA/dist, 12.f) - std::pow(SIGMA/dist, 6.f));
         if (a == NULL || b == NULL)
            return 10.0f;
         else
             return energy;
    });

    float shift_x, shift_y, shift_z;

    for (int i = 0; i < atoms_count_x; ++i) {
        shift_x  = i * shell_length;
        for (int j = 0; j < atoms_count_y; ++j) {
            shift_y = j * shell_width;
            for (int k = 0; k < atoms_count_z; ++k) {
                shift_z = k * shell_height;
                mol.Translate(glm::vec3(shift_x, shift_y, shift_z));
                cellLinkedLists->AddMol(mol);
                mol.Translate(-glm::vec3(shift_x, shift_y, shift_z));
            }
        }
    }
    std::cout << "mols_count: " << cellLinkedLists->MolsCount() << std::endl;
    cellLinkedLists->FormCellLinkedLists();
    coordVec.resize(6 * cellLinkedLists->MolsCount());
    step.resize(coordVec.size());

    for (int i = 0; i < coordVec.size(); ++i) {
        int mol_idx = i / 6;

        switch (i % 6) {
            case 0:
                coordVec[i] = cellLinkedLists->GetMol(mol_idx).GetBarycenter().x;
                step[i] = params.step_x;
                break;
            case 1:
                coordVec[i] = cellLinkedLists->GetMol(mol_idx).GetBarycenter().y;
                step[i] = params.step_y;
                break;
            case 2:
                coordVec[i] = cellLinkedLists->GetMol(mol_idx).GetBarycenter().z;
                step[i] = params.step_z;
                break;
            case 3:
                coordVec[i] = 0;
                step[i] = params.step_alpha;
                break;
            case 4:
                coordVec[i] = 0;
                step[i] = params.step_betta;
                break;
            case 5:
                coordVec[i] = 0;
                step[i] = params.step_gamma;
                break;
            default:
                break;
        }
    }
    std::cout << "init success!" << std::endl;
    this->totalDist = objectiveFunc();
    std::cout << "objective function value: " << totalDist << std::endl;
    cellLinkedLists->SaveToCSV("../resources/cll0.csv");
}

void pmols::HJPacker::Pack() {
//    this->totalDist = objectiveFunc();

    ublas::vector<float> x1_ = coordVec;
    ublas::vector<float> x2_ = x1_, x3_, x4_;
    int it_num = 1;

    std::cout << "STEP #" << it_num << std::endl;
    int search_res = exploringSearch(x2_, true); // make research without decreasing of step
    std::cout << "——————————————————————————————————————————————" << std::endl;

    do {
        std::cout << "STEP #" << ++it_num << std::endl;

        if(search_res == 1) {
            search_res = patternSearch(x1_, x2_);
        }
        else if(search_res == 0) {
            search_res = exploringSearch(x2_, true);
        }

        std::cout << "——————————————————————————————————————————————" << std::endl;
        std::string root="../resources/";
        if(it_num % 50 == 0) {
            Save(root + "lattice_"
                 + std::to_string((int)params.box_length) + "x"
                 + std::to_string((int)params.box_width) + "x"
                 + std::to_string((int)params.box_height) + "_"
                 + std::to_string(it_num) + "." + params.out_format);
            cellLinkedLists->SaveToCSV(root + "cll_" + std::to_string(it_num) + ".csv");
        }
    } while(search_res >= 0);

//    applyUVecToMols(x2_);
}

float pmols::HJPacker::eps(int coord_number) {
    return coord_number % 6 < 3 ? params.trans_eps : params.rot_eps;
}

void pmols::HJPacker::Save() {
    Save(params.out_file);
}

float pmols::HJPacker::objectiveFunc() {
    float sum = 0;
    float dist;

    for (int i = 0; i < cellLinkedLists->MolsCount(); ++i) {
        dist = cellLinkedLists->MolDist(i);
        sum += dist;
    }

    return sum;
}

int pmols::HJPacker::exploringSearch(ublas::vector<float> &x_, bool change_step) {
    std::cout << "EXPLORING SEARCH (with " << (change_step ? "step change" : "no step change") << ")" << std::endl;
    std::cout << " —— objective func before iteration: " << totalDist << std::endl;
    float sum0 = totalDist;
    float sum1, sum2;
    int mol_idx, op_num;
    float dist0;
    bool changed = false;
    bool terminate = true;
    bool mol_moved;
    float max_step_rot = 0, max_step_trans = 0;
    float min_step_rot = params.step_alpha, min_step_trans = params.step_x;

    for (int i = 0; i < x_.size(); ++i) {
        if(i % 6 < 3) {
            if(step[i] > max_step_trans)
                max_step_trans = step[i];
            else if(step[i] < min_step_trans)
                min_step_trans = step[i];
        }
        else {
            if (step[i] > max_step_rot)
                max_step_rot = step[i];
            else if(step[i] < min_step_rot)
                min_step_rot = step[i];
        }

        if(step[i] < eps(i))
            continue;

        if(terminate)
            terminate = false;

        mol_idx = i / 6;
        op_num = i % 6;
        dist0 = cellLinkedLists->MolDist(mol_idx);


        mol_moved = cellLinkedLists->MoveMol(mol_idx, (MOVE_OP)op_num, step[i]);
        sum1 = mol_moved ? ( sum0 - dist0 + cellLinkedLists->MolDist(mol_idx) ) : sum0;
        if(mol_moved)
            cellLinkedLists->CancelMove();


        mol_moved = cellLinkedLists->MoveMol(mol_idx, (MOVE_OP)op_num, -step[i]);
        sum2 = mol_moved ? ( sum0 - dist0 + cellLinkedLists->MolDist(mol_idx) ) : sum0;
        if(mol_moved)
            cellLinkedLists->CancelMove();


        if (sum0 <= std::min(sum1, sum2) && change_step)
            step[i] *= params.step_coefficient;
        else {
            if(sum1 < std::min(sum0, sum2)) {
                x_[i] += step[i];
                cellLinkedLists->MoveMol(mol_idx, (MOVE_OP)op_num, step[i]);
                totalDist = sum1;
                changed = true;
            }
            else if(sum2 < std::min(sum0, sum1)) {
                x_[i] -= step[i];
                cellLinkedLists->MoveMol(mol_idx, (MOVE_OP)op_num, -step[i]);
                totalDist = sum2;
                changed = true;
            }
        }
    }
    std::cout << "\tmax rotation step: " << max_step_rot << std::endl;
    std::cout << "\tmax translation step: " << max_step_trans << std::endl << std::endl;

    std::cout << "\tmin rotation step: " << min_step_rot << std::endl;
    std::cout << "\tmin translation step: " << min_step_trans << std::endl;

    std::cout << " —— objective func after iteration: " << totalDist << std::endl;
    if (terminate)
        return -1;

    return (int)changed;
}

int pmols::HJPacker::patternSearch(ublas::vector<float> &x1_, ublas::vector<float> &x2_) {
    std::cout << "PATTERN SEARCH" << std::endl;
//    std::cout << " —— objective func before pattern search: " << totalDist << std::endl;
    ublas::vector<float> x3_ = x1_ + params.lambda * (x2_ - x1_);
//    ublas::vector<float> dx_ = x3_ - x2_;
//    float dist;
//    int mol_idx;

    //change molecules in dx_ direction:
//    for (int i = 0; i < dx_.size(); ++i) {
//        mol_idx = i / 6;
//        dist = cellLinkedLists->MolDist(mol_idx);
//        cellLinkedLists->MoveMol(mol_idx, static_cast<MOVE_OP>(i % 6), dx_[i]);
//        totalDist += -dist + cellLinkedLists->MolDist(mol_idx);
//    }
//    std::cout << " —— objective func after pattern search: " << totalDist << std::endl;

    ublas::vector<float> x4_ = x3_;

    int exploring_res = exploringSearch(x4_, false);

    if(exploring_res == 1) {
        x1_ = x2_;
        x2_ = x4_;
    }
    else if(exploring_res == 0) {
        x1_ = x2_;
    }

    return exploring_res;
}

void pmols::HJPacker::Save(std::string out_file) {
    Atom::obatom_count = 0;

    if(params.out_format == "json") {
        rapidjson::Document jsonDoc;
        jsonDoc.SetObject();
        rapidjson::Value mols_arr(rapidjson::kArrayType);
        rapidjson::Document::AllocatorType& allocator = jsonDoc.GetAllocator();
        char b_str_buf[7];

        for (int i = 0; i < cellLinkedLists->MolsCount(); ++i) {
            rapidjson::Value mol_val(rapidjson::kObjectType);

            rapidjson::Value bonds(rapidjson::kArrayType);

            for (int j = 0; j < cellLinkedLists->GetMol(i).BondsCount(); ++j) {
                rapidjson::Value begin;
                std::string b_str = std::to_string(cellLinkedLists->GetMol(i).GetBond(j).begin->atom_idx);

                int buf_len = sprintf(b_str_buf, "%s", b_str.c_str());
                begin.SetString(b_str_buf, buf_len, allocator);
                memset(b_str_buf, 0, 7);

                rapidjson::Value end;
                end.SetInt(cellLinkedLists->GetMol(i).GetBond(j).end->atom_idx);

                rapidjson::Value bond(rapidjson::kObjectType);
                bond.AddMember(begin, end, allocator);

                bonds.PushBack(bond, allocator);
            }

            rapidjson::Value atoms(rapidjson::kObjectType);
            for (int j = 0; j < cellLinkedLists->GetMol(i).AtomsCount(); ++j) {
                char buf[4];
                int len = std::sprintf(buf, "%d", cellLinkedLists->GetMol(i).GetAtom(j).atom_idx);
                rapidjson::Value atom(rapidjson::kObjectType);

                rapidjson::Value atomic_num(cellLinkedLists->GetMol(i).GetAtom(j).atomic_number);
                atom.AddMember("atomic_num", atomic_num, allocator);

//                rapidjson::Value symbol(rapidjson::kObjectType);
//                symbol.SetString(cellLinkedLists->GetMol(i).GetAtom(j).symbol.c_str(), allocator);
//                atom.AddMember("symbol", symbol, allocator);
                rapidjson::Value position(rapidjson::kArrayType);

                position.
                        PushBack(cellLinkedLists->GetMol(i).GetAtom(j).coord.x, allocator).
                        PushBack(cellLinkedLists->GetMol(i).GetAtom(j).coord.y, allocator).
                        PushBack(cellLinkedLists->GetMol(i).GetAtom(j).coord.z, allocator);

                atom.AddMember("position", position, allocator);

                rapidjson::Value vdw_radius(cellLinkedLists->GetMol(i).GetAtom(j).vdw_radius);
                atom.AddMember("vdw_radius",vdw_radius, allocator);

                rapidjson::Value color(rapidjson::kArrayType);
                color.
                        PushBack(cellLinkedLists->GetMol(i).GetAtom(j).color.red, allocator).
                        PushBack(cellLinkedLists->GetMol(i).GetAtom(j).color.green, allocator).
                        PushBack(cellLinkedLists->GetMol(i).GetAtom(j).color.blue, allocator);
                atom.AddMember("color", color, allocator);

                rapidjson::Value atom_idx;
                atom_idx.SetString(buf, len, allocator);
                atoms.AddMember(atom_idx, atom, allocator);
            }
            mol_val.AddMember("atoms", atoms, allocator);
            mol_val.AddMember("bonds", bonds, allocator);
            mols_arr.PushBack(mol_val, allocator);
        }

        jsonDoc.AddMember("mols", mols_arr, jsonDoc.GetAllocator());
        std::ofstream ofs(out_file);
        rapidjson::OStreamWrapper osw(ofs);

        rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
        jsonDoc.Accept(writer);

        return;
    }

    std::filebuf fb;
    fb.open (out_file, std::ios::out);

    if(!fb.is_open()) {
        std::cerr << "error: Can't write to output file" << std::endl;
    }

    std::ostream out_stream(&fb);

    OpenBabel::OBConversion obConversion;
    obConversion.SetOutStream(&out_stream);
    obConversion.SetOutFormat(params.out_format.c_str(), false);

    std::shared_ptr<OpenBabel::OBMol> mol_lattice = std::make_shared<OpenBabel::OBMol>();
    int atom_b_id;
    int atom_e_id;

    for (int i = 0; i < cellLinkedLists->MolsCount(); ++i) {
        for (int j = 0; j < cellLinkedLists->GetMol(i).AtomsCount(); ++j) {
            OpenBabel::OBAtom obAtom = cellLinkedLists->GetMol(i).GetAtom(j).OBAtom();
            mol_lattice->AddAtom(obAtom);
        }

        for (int j = 0; j < cellLinkedLists->GetMol(i).BondsCount(); ++j) {
            atom_b_id = cellLinkedLists->GetMol(i).GetBond(j).begin->atom_id;
            atom_e_id = cellLinkedLists->GetMol(i).GetBond(j).end->atom_id;
            mol_lattice->AddBond(atom_b_id, atom_e_id, 1);
        }
    }

    obConversion.Write(mol_lattice.get());
    fb.close();

    std::cout << "Result: success\nMolecules packed: " << cellLinkedLists->MolsCount() << std::endl;
}
