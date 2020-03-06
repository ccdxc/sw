import { Routes, RouterModule } from '@angular/router';
import { NetworkComponent } from './network.component';

const NETWORK_ROUTER: Routes = [
  {
    path: '',
    component: NetworkComponent
  }
];

export const NetworkRoutingModule = RouterModule.forChild(NETWORK_ROUTER);
