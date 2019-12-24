import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { WidgetsModule } from 'web-app-framework';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';

import { SecurityappsComponent } from './securityapps.component';
import { SecurityappsRoutingModule } from './securityapps.route';
import { NewsecurityappComponent} from './newsecurityapp/newsecurityapp.component';

@NgModule({
  declarations: [SecurityappsComponent, NewsecurityappComponent],
  imports: [
    CommonModule,
    FormsModule,

    PrimengModule,
    MaterialdesignModule,
    WidgetsModule,
    SharedModule,
    FlexLayoutModule,
    FormsModule,
    ReactiveFormsModule,
    SharedModule,

    SecurityappsRoutingModule
  ]
})
export class SecurityappsModule { }
