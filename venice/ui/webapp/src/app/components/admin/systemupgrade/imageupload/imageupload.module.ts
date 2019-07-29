import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ImageuploadComponent } from './imageupload.component';
import { ImageuploadRoutingModule } from './imageupload.route';
import { ObjstoreService } from '@app/services/generated/objstore.service';
/**-----
 Venice UI lib-  imports
 ------------------*/
 import { SharedModule } from '@app/components/shared//shared.module';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';
 import { FlexLayoutModule } from '@angular/flex-layout';


@NgModule({
  declarations: [ImageuploadComponent],
  imports: [
    CommonModule,
    ImageuploadRoutingModule,

    PrimengModule,
    MaterialdesignModule,
    SharedModule,
    FlexLayoutModule
  ],
  providers: [
    ObjstoreService
  ]
})
export class ImageuploadModule { }
