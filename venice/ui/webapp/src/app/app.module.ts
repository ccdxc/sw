import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { HttpClientModule, HTTP_INTERCEPTORS, HttpClientXsrfModule } from '@angular/common/http';
import { FormsModule } from '@angular/forms';

import { Location, LocationStrategy, HashLocationStrategy, DatePipe, DecimalPipe } from '@angular/common';
import 'rxjs/add/operator/map';
import 'rxjs/add/operator/toPromise';
import 'rxjs/add/operator/catch';
import 'rxjs/add/operator/do';
import 'rxjs/add/observable/throw';

// Venice Widget NPM Module
import { WidgetsModule } from 'web-app-framework';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';

// Third party NPM Module
import { MomentModule } from 'angular2-moment';
import { GridsterModule } from 'angular-gridster2';
import { FlexLayoutModule } from '@angular/flex-layout';
import { NgIdleKeepaliveModule } from '@ng-idle/keepalive';


// Pensando UI services
import { ControllerService } from '@app/services/controller.service';
import { AbstractService } from '@app/services/abstract.service';
import { AuthService } from '@app/services/auth.service';
import { WorkloadService } from '@app/services/workload.service';
// import { SearchService } from '@app/services/search.service';
import { SearchService } from '@app/services/generated/search.service';
import { VeniceUIHttpInterceptor } from '@app/inteceptors/httpinterceptor.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { AlerttableService } from '@app/services/alerttable.service';
// Pensando UI components
import { AppRoutingModule } from '@app/app.routing';
import { AppComponent } from '@app/app.component';


import { BaseComponent } from '@app/components/base/base.component';
import { CommonComponent } from '@app/common.component';

// import VeniceUI modules so that they will load fast
import { CoreModule } from '@app/core';
import { SettingsModule } from '@app/components/settings';
import { DashboardModule } from '@app/components/dashboard';
import { AlertlistModule } from '@app/components/alertlist';
import { SecurityModule } from '@app/components/security';
import { NetworkModule } from '@app/components/network';
import { LoginModule } from '@app/components/login/login.module';
import { ToolbarComponent } from '@app/widgets/toolbar/toolbar.component';
import { IdleWarningComponent } from '@app/widgets/idlewarning/idlewarning.component';
import { AuthGuard } from '@app/services/authguard.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { SharedModule } from '@app/components/shared/shared.module';
import { SearchComponent } from '@app/components/search/search/search.component';
import { SearchboxComponent } from '@app/components/search/searchbox/searchbox.component';
import { SearchsuggestionsComponent } from '@app/components/search/searchsuggestions/searchsuggestions.component';
import { SearchresultComponent } from '@app/components/search/searchresult/searchresult.component';
import { GuidesearchComponent } from './components/search/guidedsearch/guidedsearch.component';
import { UIConfigsService, UIConfigsResolver } from '@app/services/uiconfigs.service';
import { RouteGuard } from '@app/services/routeguard.service';
import { AppcontentComponent } from '@app/appcontent.component';


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
    AppcontentComponent,
    BaseComponent,
    CommonComponent,
    ToolbarComponent,
    IdleWarningComponent,
    SearchComponent,
    SearchboxComponent,
    SearchsuggestionsComponent,
    SearchresultComponent,
    GuidesearchComponent
  ],
  entryComponents: [
    BaseComponent,
    IdleWarningComponent
  ],
  imports: [
    // Needs to be before other lazy loaded modules to have
    // url routing handled correctly
    AppRoutingModule,

    // VeniceUI framework
    WidgetsModule,

    // angularJS
    BrowserModule,
    BrowserAnimationsModule,
    HttpClientModule,
    HttpClientXsrfModule.withOptions({
      cookieName: 'sid',
      headerName: 'Grpc-Metadata-Csrf-Token',
    }),
    FormsModule,
    NgIdleKeepaliveModule.forRoot(),

    // VeniceUI modules
    CoreModule,
    SettingsModule,
    AlertlistModule,
    LoginModule,
    DashboardModule,
    SecurityModule,
    NetworkModule,
    SharedModule,

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
    // Generated Services
    SearchService,
    ClusterService,
    MonitoringService,
    // Route Guards
    AuthGuard,
    RouteGuard,
    // Other
    ControllerService,
    AbstractService,
    AuthService,
    WorkloadService,
    LogService,
    LogPublishersService,
    AlerttableService,
    UIConfigsService,
    UIConfigsResolver,

    { provide: LocationStrategy, useClass: HashLocationStrategy },
    { provide: HTTP_INTERCEPTORS, useClass: VeniceUIHttpInterceptor, multi: true },
  ],
  bootstrap: [AppComponent]
})
export class AppModule { }
