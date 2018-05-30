import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';

import { LoginComponent } from './login.component';
import { loginRouter } from './login.router';



@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    loginRouter
  ],
  declarations: [LoginComponent],
  exports: [LoginComponent]
})
export class LoginModule { }
