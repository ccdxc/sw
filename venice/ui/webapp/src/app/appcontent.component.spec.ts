/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import { TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { RouterTestingModule } from '@angular/router/testing';
import { FormsModule } from '@angular/forms';
import { OverlayContainer } from '@angular/cdk/overlay';


import { HttpClientTestingModule } from '@angular/common/http/testing';
import { WidgetsModule } from 'web-app-framework';

import { Store } from '@ngrx/store';
/* ---------------------------------------------------
    Venice App libraries
----------------------------------------------------- */
import { ControllerService } from './services/controller.service';

import { LogService } from './services/logging/log.service';
import { LogPublishersService } from './services/logging/log-publishers.service';
import { AuthService } from './services/auth.service';
import { AuthService as AuthServiceGen } from './services/generated/auth.service';
import { CoreModule } from '@app/core';
import { AlertlistModule } from '@app/components/alertlist';
import { LoginModule } from './components/login/login.module';
import { ToolbarComponent } from './widgets/toolbar/toolbar.component';

import { MonitoringService } from '@app/services/generated/monitoring.service';


/* ---------------------------------------------------
    Third-party libraries
----------------------------------------------------- */
import { PrimengModule } from './lib/primeng.module';
import { MaterialdesignModule } from './lib/materialdesign.module';
import { NgIdleKeepaliveModule } from '@ng-idle/keepalive';
import { AlerttableService } from '@app/services/alerttable.service';


import { SearchComponent } from '@app/components/search/search/search.component';
import { SearchboxComponent } from '@app/components/search/searchbox/searchbox.component';
import { SearchsuggestionsComponent } from '@app/components/search/searchsuggestions/searchsuggestions.component';
import { GuidesearchComponent } from '@app/components/search/guidedsearch/guidedsearch.component';

import { AppcontentComponent } from '@app/appcontent.component';
import { SearchService } from '@app/services/generated/search.service';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { SharedModule } from '@app/components/shared/shared.module';
import { ToasterComponent, ToasterItemComponent } from '@app/widgets/toaster/toaster.component';
import { ConfirmationService } from 'primeng/primeng';
import { ClusterService } from './services/generated/cluster.service';
import { MessageService } from './services/message.service';
import { HelpoverlayComponent } from './widgets/helpcontent/helpoverlay.component';
import { PortalModule } from '@angular/cdk/portal';
import { BgfileuploadComponent } from '@app/widgets/bgfileupload/bgfileupload.component';

describe('AppcontentComponent', () => {
    configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        AppcontentComponent,
        ToolbarComponent,
        SearchComponent,
        SearchboxComponent,
        SearchsuggestionsComponent,
        GuidesearchComponent,
        ToasterComponent,
        ToasterItemComponent,
        HelpoverlayComponent,
        BgfileuploadComponent
      ],
      imports: [
        // Other modules...
        HttpClientTestingModule,
        RouterTestingModule,
        FormsModule,
        PrimengModule,
        MaterialdesignModule,
        WidgetsModule,
        CoreModule,
        AlertlistModule,
        LoginModule,
        NgIdleKeepaliveModule.forRoot(),
        NoopAnimationsModule,
        SharedModule,
        PortalModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        AlerttableService,
        AuthService,
        MonitoringService,
        LogService,
        LogPublishersService,
        Store,
        SearchService,
        OverlayContainer,
        UIConfigsService,
        AuthServiceGen,
        MessageService,
        ClusterService,
      ],
    });

    });

  it('should create the app', () => {
    const fixture = TestBed.createComponent(AppcontentComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app).toBeTruthy();
  });


});
