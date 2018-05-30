import { Routes, RouterModule } from '@angular/router';
import { WorkloadComponent } from './workload.component';

const WORKLOAD_ROUTER: Routes = [
  {
    path: '',
    component: WorkloadComponent
  }
];

export const workloadRouter = RouterModule.forChild(WORKLOAD_ROUTER);
