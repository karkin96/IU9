#include <mol.h>
#include <functional>
#include <memory>
#include <unordered_set>

#ifndef COURSEWORK_CELLLINKEDLISTS_H
#define COURSEWORK_CELLLINKEDLISTS_H

#endif //COURSEWORK_CELLLINKEDLISTS_H

namespace pmols {
    class CLLNeighbourCells;

    enum MOVE_OP {
        TRANS_X,
        TRANS_Y,
        TRANS_Z,
        ROT_X,
        ROT_Y,
        ROT_Z,
    };

    typedef std::function<float(pmols::Atom*, pmols::Atom*)> DistFunc;

    class CellLinkedLists {
        friend class CLLNeighbourCells;
        friend class HJStatistics;
    public:
        CellLinkedLists() {};
<<<<<<< HEAD
        CellLinkedLists(std::vector<Molecule*> &mols, float cellLen, DistFunc distFunc);
        CellLinkedLists(float cellLen, DistFunc distFunc);
        void Set(float cellLen, DistFunc distFunc);
        void SetBoundingBox(std::tuple<glm::vec3, std::tuple<float, float, float>> box);
        std::tuple<glm::vec3, std::tuple<float, float, float>> GetBoundingBox();
        void Update();

        Molecule* GetMol(int molIdx);
        bool AddMol(Molecule &mol);
        bool MoveMol(int molIdx, MOVE_OP moveOp, float val);
        void RemoveMol(int molIdx);
=======
        CellLinkedLists(std::vector<Molecule> &mols, float cellLen, DistFunc distFunc);
        CellLinkedLists(float cellLen, DistFunc distFunc);
        void Set(float cellLen, DistFunc distFunc);
        void FormCellLinkedLists();

        Molecule &GetMol(int molIdx);
        bool AddMol(Molecule &mol);
        bool MoveMol(int molIdx, MOVE_OP moveOp, float val);
>>>>>>> 038e334a388126d42b0fb0c2c05aa260f5dd3043
        void CancelMove();
        float MolDist(int molIdx);
        void SaveToCSV(std::string file_path);

        size_t MolsCount();
        ~CellLinkedLists();
    private:
<<<<<<< HEAD
        void allocateGrid();
        void freeGrid();

        void repMol(int molIdx, Molecule mol);
        std::tuple<int, int, int> getCellIndex(Atom atom);
=======
        void calcBoundingBox();
        void repMol(int molIdx, Molecule &mol);
        std::tuple<int, int, int> getCellIndex(Atom &atom);
>>>>>>> 038e334a388126d42b0fb0c2c05aa260f5dd3043

        int cells_count_x, cells_count_y, cells_count_z;

        glm::vec3 appos_point;
        float box_length, box_width, box_height;

        float cell_len;
        DistFunc dist_func;

        bool formed;
        bool mol_moved;
<<<<<<< HEAD
        bool grid_box_setted;

        std::vector<Molecule*> mols;
=======

        std::vector<Molecule> mols;
>>>>>>> 038e334a388126d42b0fb0c2c05aa260f5dd3043
        std::list<Atom> ***atom_grid;
        std::shared_ptr<std::tuple<int, Molecule>> last_mol;
    };


    class CLLNeighbourCells {
    public:
        CLLNeighbourCells(CellLinkedLists *cellLinkedLists, Atom &atom);
        std::list<Atom> next();
        bool hasNext();
        void reset();

    private:
        CellLinkedLists *clLists;
        Atom initial_atom;

        int trans_code;
        int i0, j0, k0;
    };
}