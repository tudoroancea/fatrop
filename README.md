# FATROP
Fatrop is a constrained nonlinear optimal control problem solver that is fast and achieves a high numerical robustness.

The main features of the solver are:
- high numerical robustness by implementation of advanced numerical optimization techniques, inspired by IPOPT
- fast by exploiting the optimal control problem structure through a specialized linear solver, based on the Riccati recursion
- effecitve handling of path equality and inequality constraints, without relying on penalty methods
- ability to incorporate exact Lagrangian Hessian information
- ability to be initialized from any, possibly infeasible, solution estimate
- interfaced to rockit, which is a high-level optimal control problem specification framework, built on top of CasADi

# Installation instructions
## build and install fatrop
At this moment fatrop is only tested on linux machines.
clone the fatrop repository 

    git clone https://gitlab.kuleuven.be/robotgenskill/fatrop/fatrop.git
    cd fatrop

(optionally) checkout the branch with the most recent fatrop version:

    git checkout develop

load the blasfeo submodule

    git submodule update --recursive --init
build and install the fatrop project

    mkdir build
    cd build
    cmake -DBLASFEO_TARGET=X64_AUTOMATIC ..
    make -j
if you want to install fatrop on your system: 
    sudo make install

for non-X64 targets change the blasfeo_target parameter according to the table of https://github.com/giaf/blasfeo
## build and install fatropy
fatropy is the name of the python bindings for fatrop 
install python-dev tools (relace X.X with your python version)

    sudo apt-get install pythonX.X-dev

navigate to the fatropy source folder

    cd <fatrop_source_dir>/fatropy

tell fatropy where to find the libfatrop.so shared library file: uncomment the following line in the setup.py file and replace INSTALLATION FOLDER with the absolute path to the libfatrop.so file. 

either <fatrop_source_dir>/build/fatrop or your installation directory (for example /usr/local/lib)

        # runtime_library_dirs=["INSTALLATION FOLDER"],

alternatively you could set the LD_LIBRARY_PATH to include the folder with the libfatrop.so shared library file

build and install in your python environment

    pip install -e .

## install rockit with fatropy interface 

    git clone --recurse-submodules https://gitlab.kuleuven.be/meco-software/rockit.git 
    cd rockit
    pip install -e .

## examples 

https://gitlab.kuleuven.be/robotgenskill/fatrop/fatrop_rockit_demo

https://gitlab.kuleuven.be/robotgenskill/fatrop/fatrop_benchmarks

using fatrop from cpp:

fatrop/executables/RunFatrop.cpp

# Minimal cpp usage example for using Fatrop for OCP generated by Rockit-Fatrop interface

    #include <iostream>
    #include <fstream>
    #include <string>
    #include <ocp/StageOCPApplication.hpp>
    using namespace fatrop;
    int main(int argc, char **argv)
    {
        if (argc == 3)
        {
            //// dynamic memory allocation  
            StageOCPApplication app = StageOCPApplicationBuilder::from_rockit_interface(argv[1], argv[2]);
            auto eval_expression = app.get_expression("control_u").at_t0();
            vector<double> u0_result(eval_expression.size());
            ///  no dynamic memory allocation
            app.optimize();
            // ///  retrieve solution
            app.last_stageocp_solution().evaluate(eval_expression, u0_result);
            ///  initialize next solver run 
            app.set_initial(app.last_solution());
            ///  change solver options
            app.set_option("tol", 1e-6);
            app.optimize();
        }
        else
        {
            cout << "run me as RunFatrop f.so f.json" << endl;
        }
    }

Developer Lander Vanroye (lander.vanroye@kuleuven.be)

Thanks to all contributors:
- Wilm Decré for the cmake configuration, pip executable, pilot user instructions, fatropy python interface, ...
- Ajay Sathya for the rockit interface

