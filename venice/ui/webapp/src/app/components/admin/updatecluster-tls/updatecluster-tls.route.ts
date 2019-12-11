import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { UpdateclusterTlsComponent } from './updatecluster-tls.component';


const routes: Routes = [
    {
        path: '',
        component: UpdateclusterTlsComponent
      },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class UpdateclusterTLSRoutingModule { }
