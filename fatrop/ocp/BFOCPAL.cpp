#include "BFOCPAL.hpp"
using namespace fatrop;
int BFOCPAL::get_nxk(const int k) const
{
    return ocp_->get_nxk(k);
};

int BFOCPAL::get_nuk(const int k) const
{
    ocp_->get_nuk(k);
};

int BFOCPAL::get_ngk(const int k) const
{
    ocp_->get_ngk(k);
}

int BFOCPAL::get_n_stage_params_k(const int k) const
{
    ocp_->get_n_stage_params_k(k);
}

int BFOCPAL::get_n_global_parmas() const
{
    ocp_->get_n_global_parmas();
}

int BFOCPAL::get_ng_ineq_k(const int k) const
{
    return 0;
}
int BFOCPAL::get_ng_ineq_k_AL(const int k) const
{
    return ocp_->get_ng_ineq_k(k);
}

int BFOCPAL::get_horizon_length() const
{
    ocp_->get_horizon_length();
}

int BFOCPAL::eval_BAbtk(
    const double *states_kp1,
    const double *inputs_k,
    const double *states_k,
    const double *stage_params_k,
    const double *global_params_k,
    MAT *res,
    const int k)
{
    ocp_->eval_BAbtk(
        states_kp1,
        inputs_k,
        states_k,
        stage_params_k,
        global_params_k,
        res,
        k);
};

int BFOCPAL::eval_RSQrqtk(
    const double *objective_scale,
    const double *inputs_k,
    const double *states_k,
    const double *lam_dyn_k,
    const double *lam_eq_k,
    const double *lam_eq_ineq_k,
    const double *stage_params_k,
    const double *global_params_k,
    MAT *res,
    const int k)
{
    int no_ineqsk = no_ineqs.at(k);
    int nuk = nu.at(k);
    int nxk = nx.at(k);
    int offs = ineqs_offsets.at(k);
    double *lowerp = ((blasfeo_dvec *)lower_bounds)->pa + offs;
    double *upperp = ((blasfeo_dvec *)upper_bounds)->pa + offs;
    double *tmpviolationp = ((blasfeo_dvec *)tmpviolation)->pa;
    double *ineq_lagsp = ((blasfeo_dvec *)ineq_lags)->pa + offs;
    double *gradvecp = ((blasfeo_dvec *)gradvec)->pa;
    double *lagsupdatedp = ((blasfeo_dvec *)lagsupdated)->pa;
    double penalty = this->penalty;
    MAT *tmpmatp = (MAT *)this->tmpmat;
    // evaluate ineq Jacobian
    this->eval_Ggt_ineqk_AL(
        inputs_k,
        states_k,
        stage_params_k,
        global_params_k,
        tmpmatp,
        k);
    // evaluate ineq vector
    // todo with ROWEX
    this->eval_gineqk_AL(
        states_k,
        inputs_k,
        stage_params_k,
        global_params_k,
        tmpviolationp,
        k);
    // calculate updated lagineqs and vector for gradient
    for (int i = 0; i < no_ineqsk; i++)
    {
        double ineqlagsi = ineq_lagsp[i];
        // lambdai = 0
        if (ineqlagsi == 0)
        {
            // gradvecp[i] = 0.0;
            lagsupdatedp[i] = 0.0;
        }
        // lambdai != 0
        else
        {
            double violationi = tmpviolationp[i];
            double loweri = lowerp[i];
            double upperi = upperp[i];
            double dist_low = violationi - loweri;
            double dist_up = upperi - violationi;
            double lagineqi = ineq_lagsp[i];
            if (dist_low < 0)
            {
                lagsupdatedp[i] = -lagineqi + penalty * (std::max(0.0, -dist_low));
            }
            else if (dist_up < 0)
            {
                lagsupdatedp[i] = lagineqi - penalty * (std::max(0.0, -dist_up));
            }
            else
            {
                lagsupdatedp[i] = 0;
            }
            // gradvecp[i] =;
        }
    }
    ocp_->eval_RSQrqtk(
        objective_scale,
        inputs_k,
        states_k,
        lam_dyn_k,
        lam_eq_k,
        lagsupdatedp,
        stage_params_k,
        global_params_k,
        res,
        k);
    // TODO add extra penalty
    SYRK_LN_MN(nuk + nxk, nuk + nxk, no_ineqsk, 2 * penalty, tmpmatp, 0, 0, tmpmatp, 0, 0, 0.0, res, 0, 0, res, 0, 0);
    return 0;
};

int BFOCPAL::eval_Ggtk(
    const double *inputs_k,
    const double *states_k,
    const double *stage_params_k,
    const double *global_params_k,
    MAT *res,
    const int k)
{
    ocp_->eval_Ggtk(
        inputs_k,
        states_k,
        stage_params_k,
        global_params_k,
        res,
        k);
};

int BFOCPAL::eval_Ggt_ineqk(
    const double *inputs_k,
    const double *states_k,
    const double *stage_params_k,
    const double *global_params_k,
    MAT *res,
    const int k)
{
    return 0;
};
int BFOCPAL::eval_Ggt_ineqk_AL(
    const double *inputs_k,
    const double *states_k,
    const double *stage_params_k,
    const double *global_params_k,
    MAT *res,
    const int k)
{
    ocp_->eval_Ggt_ineqk(
        inputs_k,
        states_k,
        stage_params_k,
        global_params_k,
        res,
        k);
};

int BFOCPAL::eval_bk(
    const double *states_kp1,
    const double *inputs_k,
    const double *states_k,
    const double *stage_params_k,
    const double *global_params_k,
    double *res,
    const int k)
{
    ocp_->eval_bk(
        states_kp1,
        inputs_k,
        states_k,
        stage_params_k,
        global_params_k,
        res,
        k);
};

int BFOCPAL::eval_gk(
    const double *states_k,
    const double *inputs_k,
    const double *stage_params_k,
    const double *global_params_k,
    double *res,
    const int k)
{
    ocp_->eval_gk(
        states_k,
        inputs_k,
        stage_params_k,
        global_params_k,
        res,
        k);
};

int BFOCPAL::eval_gineqk(
    const double *states_k,
    const double *inputs_k,
    const double *stage_params_k,
    const double *global_params_k,
    double *res,
    const int k)
{
    return 0;
};
int BFOCPAL::eval_gineqk_AL(
    const double *states_k,
    const double *inputs_k,
    const double *stage_params_k,
    const double *global_params_k,
    double *res,
    const int k)
{
    ocp_->eval_gineqk(
        states_k,
        inputs_k,
        stage_params_k,
        global_params_k,
        res,
        k);
}

int BFOCPAL::eval_rqk(
    const double *objective_scale,
    const double *inputs_k,
    const double *states_k,
    const double *stage_params_k,
    const double *global_params_k,
    double *res,
    const int k)
{
    ocp_->eval_rqk(
        objective_scale,
        inputs_k,
        states_k,
        stage_params_k,
        global_params_k,
        res,
        k);
    // TODO add penalty
};

int BFOCPAL::eval_Lk(
    const double *objective_scale,
    const double *inputs_k,
    const double *states_k,
    const double *stage_params_k,
    const double *global_params_k,
    double *res,
    const int k)
{
    ocp_->eval_Lk(
        objective_scale,
        inputs_k,
        states_k,
        stage_params_k,
        global_params_k,
        res,
        k);
    // TODO add penalty
};