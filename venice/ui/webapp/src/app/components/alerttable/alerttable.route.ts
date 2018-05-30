import { Routes, RouterModule } from '@angular/router';
import { AlerttableComponent } from './alerttable.component';

const ALERTTABLE_ROUTER: Routes = [
  {
    path: '',
    component: AlerttableComponent
  }
];

export const alerttableRouter = RouterModule.forChild(ALERTTABLE_ROUTER);
