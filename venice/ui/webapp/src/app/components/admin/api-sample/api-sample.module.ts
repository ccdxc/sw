import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ApiSampleComponent } from './api-sample.component';
import { PrimengModule } from '@app/lib/primeng.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { ApiSampleRoutingModule } from './api-sample.route';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';
import { ApiCaptureComponent } from './api-capture/api-capture.component';

@NgModule({
  declarations: [ApiSampleComponent, ApiCaptureComponent],
  imports: [
    CommonModule,
    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    SharedModule,
    ApiSampleRoutingModule
  ]
})
export class ApiSampleModule { }
