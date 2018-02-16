import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { HttpClientModule, HTTP_INTERCEPTORS } from '@angular/common/http';
import { FormsModule } from '@angular/forms';

import { Location, LocationStrategy, HashLocationStrategy, DatePipe, DecimalPipe } from '@angular/common';
import 'rxjs/add/operator/map';
import 'rxjs/add/operator/toPromise';
import 'rxjs/add/operator/catch';
import 'rxjs/add/operator/do';
import 'rxjs/add/observable/throw';

// Venice Widget NPM Module
import {WidgetsModule} from 'web-app-framework';
import {MaterialdesignModule } from './lib/materialdesign.module';
import {PrimengModule} from './lib/primeng.module';

// Third party NPM Module
import { MomentModule } from 'angular2-moment';
import { GridsterModule } from 'angular2gridster';


// Pensando UI services
import { ControllerService} from './services/controller.service';
import { AbstractService } from './services/abstract.service';
import { AuthService } from './services/auth.service';
import { WorkloadService } from './services/workload.service';
import { DatafetchService } from './services/datafetch.service';
import { VeniceUIHttpInterceptor} from './inteceptors/httpinterceptor.service';
import { LogService } from './services/logging/log.service';
import { LogPublishersService } from './services/logging/log-publishers.service';

// Pensando UI components
import { routing } from './app.routing';
import { AppComponent } from './app.component';


import { BaseComponent } from './components/base/base.component';
import { CommonComponent } from './common.component';

import { CoreModule } from '@app/core';
import { SettingsModule } from './components/settings';

/**
 * This is pensando UI web-app module. It imports all required libraries.
 *
 * We use third party libraries.  Some libraries are very big, such as angular-material and primeNG.
 * We organize the big libary to app\lib\*.module.ts
 *
 * Try to keep this file small and clean
 *
 */
@NgModule({
  declarations: [
    AppComponent,
    BaseComponent,
    CommonComponent,
  ],
  imports: [

    // Venice-UI
    routing,
    // VeniceUI framework
    WidgetsModule,

    // angularJS
    BrowserModule,
    BrowserAnimationsModule,
    BrowserModule,
    HttpClientModule,
    FormsModule,

    CoreModule,
    SettingsModule,

    // material-design
    MaterialdesignModule,

    // primeNG
    PrimengModule,

    // moment.js
    MomentModule,
    // angular2gridster
    GridsterModule ,

  ],
  providers: [
    ControllerService,
    AbstractService,
    AuthService,
    WorkloadService,
    DatafetchService,
    LogService,
    LogPublishersService,
    { provide: LocationStrategy, useClass: HashLocationStrategy },
    { provide: HTTP_INTERCEPTORS, useClass: VeniceUIHttpInterceptor, multi: true }  ,
  ],
  bootstrap: [AppComponent]
})
export class AppModule { }
