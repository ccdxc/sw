import {NgModule} from '@angular/core';
import {RouterModule, Routes} from '@angular/router';
import {HostsComponent} from '@components/cluster-group/hosts/hosts.component';

const routes: Routes = [
  {
    path: '',
    component: HostsComponent
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HostsRoutingModule {
}
