import { Routes, RouterModule } from '@angular/router';
import { WorkloadComponent } from './workload.component';
import { WorkloaddetailComponent } from './workloaddetail/workloaddetail.component';

const WORKLOAD_ROUTER: Routes = [
  {
    path: '',
    component: WorkloadComponent
  },
  {
    path: ':id',
    component: WorkloaddetailComponent
  },
];

export const workloadRouter = RouterModule.forChild(WORKLOAD_ROUTER);
