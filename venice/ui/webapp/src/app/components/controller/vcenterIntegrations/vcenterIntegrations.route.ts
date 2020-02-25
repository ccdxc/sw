import {NgModule} from '@angular/core';
import {RouterModule, Routes} from '@angular/router';
import {VcenterIntegrationsComponent} from '@components/controller/vcenterIntegrations/vcenterIntegrations.component';

const routes: Routes = [
  {
    path: '',
    component: VcenterIntegrationsComponent
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class VcenterIntegrationsRoutingModule {
}
