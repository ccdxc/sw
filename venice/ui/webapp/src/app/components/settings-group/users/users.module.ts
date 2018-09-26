import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

import { UsersComponent } from './users.component';
import { UsersRoutingModule } from './users.route';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';

import { SharedModule } from '@app/components/shared//shared.module';

import { AuthService } from '@app/services/generated/auth.service';
import { NewuserComponent } from './newuser/newuser.component';


@NgModule({
  imports: [
    CommonModule,
    UsersRoutingModule,
    FormsModule,
    ReactiveFormsModule,
    FlexLayoutModule,
    PrimengModule,
    MaterialdesignModule,
    SharedModule,
  ],
  declarations: [
    UsersComponent,
    NewuserComponent
  ],
  exports: [UsersComponent],
  entryComponents: [
    UsersComponent,
    NewuserComponent
  ],
  providers: [
    AuthService
  ]
})
export class UsersModule { }
