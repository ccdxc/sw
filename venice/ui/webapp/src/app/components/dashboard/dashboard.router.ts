import { Routes, RouterModule } from '@angular/router';
import {DashboardComponent} from './dashboard.component';

const DASHBOARD_ROUTER: Routes = [
    {
        path: '',
        component: DashboardComponent
    }
];

export const dashboardRouter = RouterModule.forChild(DASHBOARD_ROUTER );
