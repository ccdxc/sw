import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { HttpClientModule, HTTP_INTERCEPTORS, HttpClientXsrfModule } from '@angular/common/http';
import { FormsModule } from '@angular/forms';

import { LocationStrategy, HashLocationStrategy } from '@angular/common';






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
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { StagingService } from '@app/services/generated/staging.service';
import { WorkloadService } from '@app/services/workload.service';
import { WorkloadService as WorkloadServiceGen } from '@app/services/generated/workload.service';
import { SearchService } from '@app/services/generated/search.service';
import { VeniceUIHttpInterceptor } from '@app/inteceptors/httpinterceptor.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { AlerttableService } from '@app/services/alerttable.service';
import { TelemetryqueryService as TelemetryqueryServiceGen } from '@app/services/generated/telemetryquery.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { NetworkService} from '@app/services/generated/network.service';
import { BrowserService } from '@app/services/generated/browser.service';
import { OrchestrationService } from '@app/services/generated/orchestration.service';
import { FwlogService } from '@app/services/generated/fwlog.service';

// Pensando UI components
import { AppRoutingModule } from '@app/app.routing';
import { AppComponent } from '@app/app.component';

// import VeniceUI modules so that they will load fast
import { CoreModule } from '@app/core';
import { SettingsGroupModule } from '@app/components/settings-group/settings-group.module';
import { DashboardModule } from '@app/components/dashboard';
import { AlertlistModule } from '@app/components/alertlist';
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
import { SecurityService } from '@app/services/generated/security.service';
import { EventsService } from '@app/services/events.service';
import { ConfirmationService, DomHandler } from 'primeng/primeng';
import { ToasterComponent, ToasterItemComponent } from './widgets/toaster/toaster.component';
import { MissingpageComponent } from './widgets/missingpage/missingpage.component';
import { MessageService } from './services/message.service';
import {AuditService} from '@app/services/generated/audit.service';
import { HelpoverlayComponent } from './widgets/helpcontent/helpoverlay.component';
import { DragDropModule } from '@angular/cdk/drag-drop';
import { PortalModule } from '@angular/cdk/portal';
import { BgfileuploadComponent } from './widgets/bgfileupload/bgfileupload.component';
import { RolloutService } from '@app/services/generated/rollout.service';
import { RolloutsModule } from './components/admin/systemupgrade/rollouts/rollouts.module';
import { LoginModule } from './components/login/login.module';
import { LicenseService } from './services/license.service';

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
    ToolbarComponent,
    IdleWarningComponent,
    SearchComponent,
    SearchboxComponent,
    SearchsuggestionsComponent,
    SearchresultComponent,
    GuidesearchComponent,
    ToasterItemComponent,
    ToasterComponent,
    MissingpageComponent,
    HelpoverlayComponent,
    BgfileuploadComponent,
  ],
  entryComponents: [
    IdleWarningComponent,
    HelpoverlayComponent
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
    SettingsGroupModule,
    AlertlistModule,
    SharedModule,

    // material-design
    MaterialdesignModule,

    // primeNG
    PrimengModule,

    // moment.js
    MomentModule,
    GridsterModule,
    FlexLayoutModule,
    DragDropModule,
    PortalModule,
    RolloutsModule,
    LoginModule,
  ],
  providers: [
    // Generated Services
    SearchService,
    ClusterService,
    MonitoringService,
    SecurityService,
    WorkloadServiceGen,
    AuthServiceGen,
    StagingService,
    TelemetryqueryServiceGen,
    MetricsqueryService,
    AuditService,
    RolloutService,

    // Route Guards
    AuthGuard,
    RouteGuard,
    // Other
    ControllerService,
    EventsService,
    AbstractService,
    AuthService,
    WorkloadService,
    LogService,
    LogPublishersService,
    AlerttableService,
    UIConfigsService,
    UIConfigsResolver,
    EventsService,
    MessageService,
    ConfirmationService,
    NetworkService,
    BrowserService,
    OrchestrationService,
    FwlogService,
    LicenseService,

    { provide: LocationStrategy, useClass: HashLocationStrategy },
    { provide: HTTP_INTERCEPTORS, useClass: VeniceUIHttpInterceptor, multi: true },
  ],
  bootstrap: [AppComponent]
})
export class AppModule { }
