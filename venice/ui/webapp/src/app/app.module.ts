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
import {MaterialdesignModule } from '@app/lib/materialdesign.module';
import {PrimengModule} from '@app/lib/primeng.module';

// Third party NPM Module
import { MomentModule } from 'angular2-moment';
import { GridsterModule } from 'angular-gridster2';
import { FlexLayoutModule } from '@angular/flex-layout';
import { NgIdleKeepaliveModule } from '@ng-idle/keepalive';


// Pensando UI services
import { ControllerService} from '@app/services/controller.service';
import { AbstractService } from '@app/services/abstract.service';
import { AuthService } from '@app/services/auth.service';
import { WorkloadService } from '@app/services/workload.service';
import { DatafetchService } from '@app/services/datafetch.service';
import { VeniceUIHttpInterceptor} from '@app/inteceptors/httpinterceptor.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { AlerttableService } from '@app/services/alerttable.service';
import { SecurityService } from '@app/services/security.service';
// Pensando UI components
import { routing } from '@app/app.routing';
import { AppComponent } from '@app/app.component';


import { BaseComponent } from '@app/components/base/base.component';
import { CommonComponent } from '@app/common.component';

// import VeniceUI modules so that they will load fast
import { CoreModule } from '@app/core';
import { SettingsModule } from '@app/components/settings';
import { DashboardModule } from '@app/components/dashboard';
import { AlertlistModule } from '@app/components/alertlist';
import { AlerttableModule} from '@app/components/alerttable';
import { SecurityModule } from '@app/components/security';
import { NetworkModule } from '@app/components/network';
import { ClusterModule } from '@app/components/cluster';
import { LoginModule } from '@app/components/login/login.module';
import { ToolbarComponent } from '@app/widgets/toolbar/toolbar.component';
import { IdleWarningComponent } from '@app/widgets/idlewarning/idlewarning.component';


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
    ToolbarComponent,
    IdleWarningComponent
  ],
  entryComponents: [
    BaseComponent,
    IdleWarningComponent
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
    NgIdleKeepaliveModule.forRoot(),

    // VeniceUI modules
    CoreModule,
    SettingsModule,
    AlertlistModule,
    AlerttableModule,
    LoginModule,
    DashboardModule,
    SecurityModule,
    NetworkModule,

    // material-design
    MaterialdesignModule,

    // primeNG
    PrimengModule,

    // moment.js
    MomentModule,
    GridsterModule,
    FlexLayoutModule,

  ],
  providers: [
    ControllerService,
    AbstractService,
    AuthService,
    WorkloadService,
    DatafetchService,
    LogService,
    LogPublishersService,
    AlerttableService,
    SecurityService,

    { provide: LocationStrategy, useClass: HashLocationStrategy },
    { provide: HTTP_INTERCEPTORS, useClass: VeniceUIHttpInterceptor, multi: true }  ,
  ],
  bootstrap: [AppComponent]
})
export class AppModule { }
