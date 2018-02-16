import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { Routes, RouterModule } from '@angular/router';

import {LoginComponent} from './login.component';
import {loginRouter} from './login.router';



@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    loginRouter
  ],
  declarations: [LoginComponent]
})
export class LoginModule { }
