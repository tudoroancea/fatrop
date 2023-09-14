#include "fatrop-casadi-problem.hpp"
#include "fatrop-casadi-solver.hpp"
#include "stage-problem.hpp"
#include "method-fatrop.hpp"
#include <casadi/casadi.hpp>
#include "ocp/StageOCPApplication.hpp"
#include <limits>
#include "utilities.hpp"
#include "ocp.hpp"
#define INF std::numeric_limits<double>::infinity()
using namespace fatrop;
using namespace fatrop::fatrop_casadi;
int main()
{
    // auto x = casadi::MX::sym("x", 2);
    // auto u = casadi::MX::sym("u", 2);
    // std::cout << ConstraintHelper(x<u).g_ineq << std::endl;
    auto ocp = Ocp();
    auto stage = ocp.stage();
    auto x = stage -> state(2);
    auto u = stage -> control();
    auto e = 1. - x(0) * x(0);
    double dt = .5;
    stage ->set_next(x, x + casadi::MX::vertcat({x(1), e * x(1) - x(0) + u}) * dt);
    stage ->subject_to(stage ->at_t0(x(0)) == 1.0);
    stage ->subject_to(stage ->at_t0(x(1)) == 0.0);
    stage ->subject_to(-0.25 <x(1), false, true);
    stage ->subject_to((-1.0 <u)< 1, false, false);

    stage ->add_objective(stage ->sum(u(0) * u(0) + x(0) * x(0) + x(1) * x(1), true, false));
    stage ->add_objective(stage ->at_tf(x(1) * x(1)));

    auto fatrop_method = std::make_shared<StageProblemFatropMethod>(stage.get());
    stage ->method = fatrop_method;
    fatrop_method -> transcribe(20);

    auto solver = std::make_shared<FatropCasadiSolver>(*fatrop_method);

    auto fatrop_solver = OCPApplication(solver);
    fatrop_solver.build();
    fatrop_solver.optimize();

    // typedef std::vector<double> vd;
    // // // define variables
    // auto x = casadi::MX::sym("x", 2);
    // auto u = casadi::MX::sym("u", 1);
    // auto p_stage = casadi::MX::sym("p_stage", 0);
    // auto p_global = casadi::MX::sym("p_global", 0);
    // auto e = 1. - x(0) * x(0);
    // double dt = .5;
    // auto obj = u * u + x(0) * x(0) + x(1) * x(1);
    // auto obj_term = x(1) * x(1);
    // auto x_next = x + casadi::MX::vertcat({x(1), e * x(1) - x(0) + u}) * dt;
    // auto eq_initial = cs::MX::vertcat({x(0) -1.0, x(1)});
    // auto stage_initial = MicroStage::create(MicroStageSyms{x, u, p_stage, p_global}, obj, x_next, eq_initial, cs::MX(), std::vector<double>(), std::vector<double>());
    // auto stage_middle = MicroStage::create(MicroStageSyms{x, u, p_stage, p_global}, obj, x_next, cs::MX(), cs::MX::vertcat({x(1),u}), vd{-0.25, -1} , vd{INF, 1});
    // auto stage_terminal = MicroStage::create(MicroStageSyms{x, cs::MX(), p_stage, p_global}, obj_term, cs::MX(), cs::MX(), x(1), vd{-0.25} , vd{INF});

    // // set up the problem
    // auto problem = FatropCasadiProblem();
    // // add the microstages
    // problem.push_back(stage_initial);
    // for (int i = 0; i < 19; i++)
    // {
    //     problem.push_back(stage_middle);
    // }
    // problem.push_back(stage_terminal);

    // // // set up the solver
    // auto solver = std::make_shared<FatropCasadiSolver>(problem);
    // auto fatrop_solver = OCPApplication(solver);
    // fatrop_solver.build();
    // fatrop_solver.optimize();
    return 0;
}