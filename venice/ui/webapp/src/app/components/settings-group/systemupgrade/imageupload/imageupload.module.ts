import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ImageuploadComponent } from './imageupload.component';
import { ImageuploadRoutingModule } from './imageupload.route';

/**-----
 Venice UI lib-  imports
 ------------------*/
 import { SharedModule } from '@app/components/shared//shared.module';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';


@NgModule({
  declarations: [ImageuploadComponent],
  imports: [
    CommonModule,
    ImageuploadRoutingModule,

    PrimengModule,
    MaterialdesignModule,
    SharedModule
  ]
})
export class ImageuploadModule { }
