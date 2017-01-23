//
// Created by alex on 08.11.16.
//

#include <mol.h>

std::string vec_to_string(glm::vec3 vec) {
    return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
}

int Molecule::molecules_count = 0;

int Atom::atoms_number = 0;
int Atom::proto_nums = 0;

Molecule::Molecule(std::string file_path) {
    mol_id = molecules_count;
    molecules_count++;

    OpenBabel::OBConversion conversion;

    size_t dot_pos = file_path.find_last_of(".");
    std::string file_format = file_path.substr(dot_pos + 1, file_path.length() - dot_pos);
    conversion.SetInFormat(file_format.c_str());

    OpenBabel::OBElementTable table;
    OpenBabel::OBMol mol = OpenBabel::OBMol();

    conversion.ReadFile(&mol, file_path);

    atoms_count = mol.NumAtoms();
    bonds_count = mol.NumBonds();

    atoms = new Atom[atoms_count];
    bonds = new Bond[bonds_count];

    FOR_ATOMS_OF_MOL(atom, mol) {
        int idx = atom->GetIdx() - 1;
        int atomic_num = atom->GetAtomicNum();
        atoms[idx].atomic_number = atomic_num;
        atoms[idx].color = Colorf(table.GetRGB(atomic_num));
        atoms[idx].symbol = table.GetSymbol(atomic_num);
        atoms[idx].coord =  glm::vec3(atom->GetX(), atom->GetY(), atom->GetZ());
        atoms[idx].vdw_radius = (float)table.GetVdwRad(atomic_num);
        atoms[idx].radius = atoms[idx].vdw_radius / 5.0f;
        atoms[idx].parent_mol_id = mol_id;
    }

    int i = 0;
    FOR_BONDS_OF_MOL(bond, mol) {
        int begin_idx = bond->GetBeginAtom()->GetIdx() - 1;
        int end_idx = bond->GetEndAtom()->GetIdx() - 1;

        bonds[i].begin = &atoms[begin_idx];
        bonds[i].end = &atoms[end_idx];
        bonds[i].length = (float)bond->GetLength();
        i++;
    }

    bar_vec = GetBarycenter();
}

int Molecule::AtomsCount() {
    return atoms_count;
}

int Molecule::BondsCount() {
    return bonds_count;
}

Atom &Molecule::GetAtom(int idx) {
    if(idx < 0)
        throw std::out_of_range("index can't be negative");
    else if(idx > atoms_count)
        throw std::out_of_range("idx: " + std::to_string(idx) + "\tatoms count: " + std::to_string(atoms_count));
    return atoms[idx];
}

Bond &Molecule::GetBond(int idx) {
    if(idx < 0)
        throw std::out_of_range("index can't be negative");
    else if(idx > bonds_count)
        throw std::out_of_range("idx: " + std::to_string(idx) + "\tatoms count: " + std::to_string(atoms_count));
    return bonds[idx];
}

Molecule::~Molecule() {
    if (atoms != NULL)
        delete []atoms;

    if (bonds != NULL)
        delete []bonds;
}

std::string Molecule::GetFormula() {
    return mol.GetFormula();
}

glm::vec3 Molecule::GetBarycenter() {
    glm::vec3 sum_p(0, 0, 0);

    for (int i = 0; i < atoms_count; ++i) {
        sum_p.x += atoms[i].coord.x;
        sum_p.y += atoms[i].coord.y;
        sum_p.z += atoms[i].coord.z;
    }

    return glm::vec3(sum_p.x/atoms_count, sum_p.y/atoms_count, sum_p.z/atoms_count);
}

void Molecule::RotateX(float alpha) {
    RotateOn(bar_vec, alpha, glm::vec3(1, 0, 0));
}

void Molecule::RotateY(float beta) {
    RotateOn(bar_vec, beta, glm::vec3(0, 1, 0));
}

void Molecule::RotateZ(float gamma) {
    RotateOn(bar_vec, gamma, glm::vec3(0, 0, 1));
}

void Molecule::Translate(glm::vec3 v) {
    for (int i = 0; i < atoms_count; ++i) {
        atoms[i].coord.x += v.x;
        atoms[i].coord.y += v.y;
        atoms[i].coord.z += v.z;
    }
    bar_vec.x += v.x;
    bar_vec.y += v.y;
    bar_vec.z += v.z;
}

void Molecule::RotateOn(glm::vec3 point, float angle, glm::vec3 dir) {
    glm::mat4 m_rot(1.0);
    m_rot = glm::rotate(m_rot, angle, dir);

    for (int i = 0; i < atoms_count; ++i) {
        if(atoms[i].coord != point) {
            glm::vec4 cur_vec4(atoms[i].coord.x - point.x, atoms[i].coord.y - point.y, atoms[i].coord.z - point.z, 0);
            cur_vec4 = cur_vec4 * m_rot;
            atoms[i].coord.x = point.x + cur_vec4.x;
            atoms[i].coord.y = point.y + cur_vec4.y;
            atoms[i].coord.z = point.z + cur_vec4.z;
        }
    }
}

Molecule::Molecule(const Molecule &other) {
//    std::cout << "mol copy" << std::endl;
    mol_id = molecules_count;
    molecules_count++;
    atoms_count = other.atoms_count;
    bonds_count = other.bonds_count;
    bar_vec = other.bar_vec;
    mol = other.mol;

    atoms = new Atom[atoms_count];
    bonds = new Bond[bonds_count];
    std::map<Atom*, Atom*> other_current;

    for (int i = 0; i < atoms_count; ++i) {
        atoms[i] = other.atoms[i];
        atoms[i].parent_mol_id = mol_id;

        other_current[other.atoms + i] = atoms + i;
    }

    for (int i = 0; i < bonds_count; ++i) {
        Atom *b_atom = other_current[other.bonds[i].begin];
        Atom *e_atom = other_current[other.bonds[i].end];
        bonds[i].begin = b_atom;
        bonds[i].end = e_atom;
        bonds[i].length = other.bonds[i].length;
    }
}

OpenBabel::OBMol Molecule::OBMol() {
    OpenBabel::OBMol cur_mol;
    OpenBabel::OBBond cur_bond;

    for (int i = 0; i < atoms_count; ++i) {
        OpenBabel::OBAtom cur_atom = atoms[i].OBAtom();
        cur_atom.SetParent(&cur_mol);
        cur_mol.AddAtom(cur_atom);
    }

    return cur_mol;
}

int Molecule::GetMolId() {
    return mol_id;
}

boost::tuple<glm::vec3, boost::tuple<float, float, float>> Molecule::GetRectangularShell() {
    float min_x = GetAtom(0).coord.x - GetAtom(0).vdw_radius,
            max_x = min_x + GetAtom(0).vdw_radius;

    float min_y = GetAtom(0).coord.y - GetAtom(0).vdw_radius,
            max_y = min_y + GetAtom(0).vdw_radius;

    float min_z = GetAtom(0).coord.z - GetAtom(0).vdw_radius,
            max_z = min_z + GetAtom(0).vdw_radius;

    float cur_x, cur_y, cur_z;
    float cur_r;

    for(int i = 0; i < AtomsCount(); i++) {
        cur_x = GetAtom(i).coord.x;
        cur_y = GetAtom(i).coord.y;
        cur_z = GetAtom(i).coord.z;

        cur_r = GetAtom(i).vdw_radius;

        if(cur_x - cur_r < min_x) {
            min_x = cur_x - cur_r;
        }
        else if(cur_x + cur_r > max_x) {
            max_x = cur_x + cur_r;
        }

        if(cur_y - cur_r < min_y) {
            min_y = cur_y - cur_r;
        }
        else if(cur_y + cur_r > max_y) {
            max_y = cur_y + cur_r;
        }

        if(cur_z - cur_r < min_z) {
            min_z = cur_z - cur_r;
        }
        else if(cur_z + cur_r > max_z) {
            max_z = cur_z + cur_r;
        }
    }

    boost::tuple<float, float, float> sizes(std::fabs(max_x - min_x), std::fabs(max_y - min_y), std::fabs(max_z - min_z));
    glm::vec3 appos_point(min_x, min_y, min_z);

    return boost::make_tuple(appos_point, sizes);
}


void Atom::print() {
    std::cout << "color: (" << color.red << ", " << color.green << ", " << color.blue << ")" << std::endl;
    std::cout << "symbol: " << symbol << std::endl;
    std::cout << "vdw_radius: " << vdw_radius << std::endl;
    std::cout << "coord: (" << coord.x << ", " <<  coord.y << ", " << coord.z << ")" << std::endl;
    std::cout << "radius: " << radius << std::endl;
}


void Bond::print() {
    std:: cout << begin->symbol << " -- " << end->symbol << ": " << length << std::endl;
}


bool Colorf::operator==(const Colorf &other) {
    return this->red == other.red && this->blue == other.blue && this->green == other.green;
}

