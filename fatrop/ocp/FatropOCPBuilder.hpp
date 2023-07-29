/*
 * Fatrop - A fast trajectory optimization solver
 * Copyright (C) 2022, 2023 Lander Vanroye, KU Leuven. All rights reserved.
 *
 * This file is part of Fatrop.
 *
 * Fatrop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fatrop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Fatrop.  If not, see <http://www.gnu.org/licenses/>. */
#ifndef FATROPOCPBUILDERINCLUDED
#define FATROPOCPBUILDERINCLUDED
#include "ocp/OCP.hpp"
#include "ocp/FatropOCP.hpp"
#include "ocp/OCPAbstract.hpp"
#include "ocp/OCPAdapter.hpp"
#include "ocp/OCPLSRiccati.hpp"
#include "ocp/OCPNoScaling.hpp"
namespace fatrop
{
    class FatropOCPBuilder
    {
    public:
        FatropOCPBuilder(const std::shared_ptr<OCPAbstract> &ocp, const std::shared_ptr<FatropOptions> &fatropparams, const std::shared_ptr<FatropPrinter> &printer) : ocp_(ocp), fatropoptions_(fatropparams), fatropprinter_(printer)
        {
        }
        std::shared_ptr<FatropOCP> build()
        {
            std::shared_ptr<OCPAdapter> adapter = std::make_shared<OCPAdapter>(ocp_, fatropoptions_);
            return build(adapter);
        }

        std::shared_ptr<FatropOCP> build(std::shared_ptr<OCPAdapter> &adapter)
        {
            return std::make_shared<FatropOCP>(adapter, std::make_shared<OCPLSRiccati>(adapter->get_ocp_dims(), fatropoptions_, fatropprinter_), std::make_shared<OCPNoScaling>(fatropoptions_), fatropoptions_, fatropprinter_);
        }

    private:
        const std::shared_ptr<OCPAbstract> ocp_;
        const std::shared_ptr<FatropOptions> fatropoptions_;
        const std::shared_ptr<FatropPrinter> fatropprinter_;
    };
}

#endif // !OCPALGBUILDERINCLUDED