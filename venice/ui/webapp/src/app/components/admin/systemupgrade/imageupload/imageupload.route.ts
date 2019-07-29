import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { ImageuploadComponent } from '@app/components/admin/systemupgrade/imageupload/imageupload.component';


const routes: Routes = [
  {
    path: '',
    component: ImageuploadComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ImageuploadRoutingModule { }
