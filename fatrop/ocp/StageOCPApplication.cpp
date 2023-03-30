#include "ocp/StageOCPApplication.hpp"
using namespace fatrop;
NLPApplication::NLPApplication() : fatropoptions_(make_shared<FatropOptions>()), journaller_(make_shared<Journaller>(fatropoptions_->maxiter + 1)){};

void NLPApplication::Build(const shared_ptr<FatropNLP> &nlp)
{
    // keep nlp around for getting nlpdims
    nlp_ = nlp;
    AlgBuilder algbuilder;
    algbuilder.BuildFatropAlgObjects(nlp, fatropoptions_, fatropdata_, journaller_);
    fatropalg_ = algbuilder.BuildAlgorithm();
    dirty = false;
}

int NLPApplication::Optimize()
{
    assert(!dirty);
    int ret = fatropalg_->Optimize();
    return ret;
}
// TODO: make this protected and use last_solution instead and choose other name
FatropVecBF &NLPApplication::last_solution_primal()
{
    assert(!dirty);
    return fatropdata_->x_curr;
}
FatropVecBF &NLPApplication::initial_guess_primal()
{
    assert(!dirty);
    return fatropdata_->x_initial;
}
FatropStats NLPApplication::get_stats()
{
    return fatropalg_->GetStats();
}
NLPDims NLPApplication::get_nlp_dims()
{
    return nlp_->GetNLPDims();
}
FatropVecBF &NLPApplication::last_solution_dual()
{
    return fatropdata_->lam_curr;
}
FatropVecBF &NLPApplication::last_solution_zL()
{
    return fatropdata_->zL_curr;
}
FatropVecBF &NLPApplication::last_solution_zU()
{
    return fatropdata_->zU_curr;
}
FatropVecBF &NLPApplication::initial_guess_dual()
{
    return fatropdata_->lam_init;
}
FatropVecBF &NLPApplication::initial_guess_zL()
{
    return fatropdata_->zL_init;
}
FatropVecBF &NLPApplication::initial_guess_zU()
{
    return fatropdata_->zU_init;
}
template <typename T>
void NLPApplication::set_option(const string &option_name, T value)
{
    fatropoptions_->Set(option_name, value);
}
template void NLPApplication::set_option<int>(const string&, int);
template void NLPApplication::set_option<double>(const string&, double);
template void NLPApplication::set_option<bool>(const string&, bool);


void NLPApplication::set_initial(const FatropSolution &initial_guess)
{
    initial_guess_primal() = initial_guess.sol_primal_;
    initial_guess_dual() = initial_guess.sol_dual_;
    initial_guess_zL() = initial_guess.sol_zL_;
    initial_guess_zU() = initial_guess.sol_zU_;
}
const FatropOptions &NLPApplication::get_options() const
{
    return *fatropoptions_;
}

// TODO move this class to a separate file
OCPApplication::OCPApplication(const shared_ptr<OCPAbstract> &ocp) : ocp_(ocp)
{
}

void OCPApplication::build()
{
    // keep the adapter around for accessing the parameters for samplers and parameter setters
    adapter = make_shared<OCPAdapter>(ocp_);
    shared_ptr<FatropNLP> nlp(FatropOCPBuilder(ocp_, fatropoptions_).Build(adapter));
    NLPApplication::Build(nlp);
    dirty = false;
}

vector<double> &OCPApplication::global_parameters()
{
    assert(!dirty);
    return adapter->GetGlobalParamsVec();
}
vector<double> &OCPApplication::stage_parameters()
{
    assert(!dirty);
    return adapter->GetStageParamsVec();
}
void OCPApplication::set_initial(vector<double> &initial_u, vector<double> &initial_x)
{
    assert(!dirty);
    adapter->SetInitial(fatropdata_, initial_u, initial_x);
}
OCPDims OCPApplication::get_ocp_dims()
{
    return adapter->GetOCPDims();
}

FatropSolution::FatropSolution(){};
void FatropSolution::set_dims(const NLPDims &dims)
{
    sol_primal_.resize(dims.nvars);
    sol_dual_.resize(dims.neqs);
    sol_zL_.resize(dims.nineqs);
    sol_zU_.resize(dims.nineqs);
};
void FatropSolution::set_solution(const FatropVecBF &sol_primal, const FatropVecBF &sol_dual, const FatropVecBF &sol_zL, const FatropVecBF &sol_zU)
{
    sol_primal.copyto(sol_primal_);
    sol_dual.copyto(sol_dual_);
    sol_zL.copyto(sol_zL_);
    sol_zU.copyto(sol_zU_);
};
void FatropSolution::set_primal_solution(const FatropVecBF &sol)
{
    sol.copyto(sol_primal_);
}
// StageOCPApplicationAbstract::StageOCPApplicationAbstract(const shared_ptr<StageOCP> &ocp) : OCPApplication(ocp)
// {
// }
StageOCPSolution::StageOCPSolution(const shared_ptr<OCP> &app)
{
    set_dims(app->GetOCPDims());
}
StageOCPSolution::StageOCPSolution(){};
void StageOCPSolution::set_dims(const OCPDims &dims)
{
    FatropSolution::set_dims(dims);
    nx = dims.nx.at(0);
    nu = dims.nu.at(0);
    n_stage_params = dims.n_stage_params.at(0);
    n_global_params = dims.n_global_params;
    K = dims.K;
    global_params.resize(n_global_params);
    stage_params.resize(n_stage_params);
}
void StageOCPSolution::set_parameters(const vector<double> &global_params, const vector<double> &stage_params)
{
    this->global_params = global_params;
    this->stage_params = stage_params;
}

void StageOCPSolution::sample(const StageControlGridSampler &sampler, vector<double> &result)
{
    sampler.evaluate(sol_primal_, global_params, stage_params, result);
}
vector<double> StageOCPSolution::evaluate(const StageExpressionEvaluatorBase &evaluator) const
{
    return evaluator.evaluate(sol_primal_, global_params, stage_params);
}
void StageOCPSolution::evaluate(const StageExpressionEvaluatorBase &evaluator, vector<double> &result) const
{
    evaluator.evaluate(sol_primal_, global_params, stage_params, result);
}
StageOCPApplication::StageOCPApplication(const shared_ptr<StageOCP> &ocp) : OCPApplication(ocp), nx_(ocp->nx_), nu_(ocp->nu_), n_stage_params_(ocp->n_stage_params_), K_(ocp->K_){};

StageOCPApplication::AppParameterSetter::AppParameterSetter(const shared_ptr<OCPAdapter> &adapter, const shared_ptr<ParameterSetter> &ps) : ParameterSetter(*ps), adapter_(adapter){};
void StageOCPApplication::AppParameterSetter::set_value(const double value[])
{
    ParameterSetter::set_value(adapter_->GetGlobalParamsVec(), adapter_->GetStageParamsVec(), value);
};

StageExpressionEvaluatorFactory StageOCPApplication::get_expression(const string &sampler_name)
{
    return get_expression_evaluator(stage_expressions[sampler_name]);
}
StageOCPApplication::AppParameterSetter StageOCPApplication::get_parameter_setter(const string &setter_name)
{
    return AppParameterSetter(adapter, param_setters[setter_name]);
}
void StageOCPApplication::build()
{
    OCPApplication::build();
    // allocate the last solution
    last_solution_.set_dims(get_ocp_dims());
    dirty = false;
}
int StageOCPApplication::optimize()
{
    int ret = NLPApplication::Optimize();
    last_solution_.set_parameters(global_parameters(), stage_parameters());
    if (ret == 0)
    {
        last_solution_.set_solution(last_solution_primal(), last_solution_dual(), last_solution_zL(), last_solution_zU());
    }
    return ret;
}
const StageOCPSolution &StageOCPApplication::last_stageocp_solution()
{
    return last_solution_;
}

StageOCPApplication StageOCPApplicationBuilder::from_rockit_interface(const string &functions, const string &json_spec_file)
{
    shared_ptr<DLHandler> handle = make_shared<DLHandler>(functions);
    std::ifstream t(json_spec_file);
    std::stringstream buffer;
    buffer << t.rdbuf();
    json::jobject json_spec = json::jobject::parse(buffer.str());
    auto stageocp = StageOCPBuilder::FromRockitInterface(handle, json_spec);
    // instantiate the BasicOCPApplication
    auto result = StageOCPApplication(stageocp);
    // add all samplers
    vector<string> sampler_names = json_spec["samplers"];
    // const int nu = stageocp->nu_;
    // const int nx = stageocp->nx_;
    const int no_stage_params = stageocp->n_stage_params_;
    const int K = stageocp->K_;
    for (auto sampler_name : sampler_names)
    {
        auto eval = make_shared<EvalCasGen>(handle, "sampler_" + sampler_name);
        result.stage_expressions[sampler_name] = make_shared<EvalBaseSE>(eval);
    }
    // add state samplers
    json::jobject states_offset = json_spec["states_offset"];
    vector<string> state_names = states_offset.keys();
    for (auto state_name : state_names)
    {
        vector<int> in = states_offset[state_name].as_object().array(0).get_number_array<int>("%d");
        vector<int> out = states_offset[state_name].as_object().array(1).get_number_array<int>("%d");
        result.stage_expressions[string("state_") + state_name] = make_shared<IndexEpression>(false, in, out);
    }
    // add control samplers
    json::jobject controls_offset = json_spec["controls_offset"];
    vector<string> control_names = controls_offset.keys();
    for (auto control_name : control_names)
    {
        vector<int> in = controls_offset[control_name].as_object().array(0).get_number_array<int>("%d");
        vector<int> out = controls_offset[control_name].as_object().array(1).get_number_array<int>("%d");
        result.stage_expressions[string("control_") + control_name] = make_shared<IndexEpression>(true, in, out);
    }
    // add all parameter setters
    json::jobject control_params_offset = json_spec["control_params_offset"];
    vector<string> control_params_names = control_params_offset.keys();
    for (auto control_params_name : control_params_names)
    {
        vector<int> in = control_params_offset[control_params_name].as_object().array(0).get_number_array<int>("%d");
        vector<int> out = control_params_offset[control_params_name].as_object().array(1).get_number_array<int>("%d");
        result.param_setters[control_params_name] = make_shared<ParameterSetter>(in, out, no_stage_params, in.size(), K, false);
    }
    json::jobject global_params_offset = json_spec["global_params_offset"];
    vector<string> global_params_names = global_params_offset.keys();
    for (auto global_params_name : global_params_names)
    {
        vector<int> in = global_params_offset[global_params_name].as_object().array(0).get_number_array<int>("%d");
        vector<int> out = global_params_offset[global_params_name].as_object().array(1).get_number_array<int>("%d");
        result.param_setters[global_params_name] = make_shared<ParameterSetter>(in, out, no_stage_params, in.size(), K, true);
    }
    result.build();
    return result;
}
void StageOCPApplication::AppParameterSetter::set_value(const initializer_list<double> il_)
{
    assert((int)il_.size() == _no_var);
    set_value(il_.begin());
}
StageExpressionEvaluatorFactory StageOCPApplication::get_expression_evaluator(const shared_ptr<StageExpression> &expr)
{
    return StageExpressionEvaluatorFactory(expr, nu_, nx_, n_stage_params_, K_);
}