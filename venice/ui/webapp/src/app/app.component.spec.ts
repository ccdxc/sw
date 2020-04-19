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
import { AppComponent } from '@app/app.component';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';

import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { AuthService } from '@app/services/auth.service';
import { CoreModule } from '@app/core';
import { AlertlistModule } from '@app/components/alertlist';
import { LoginModule } from '@app/components/login/login.module';
import { ToolbarComponent } from '@app/widgets/toolbar/toolbar.component';

/* ---------------------------------------------------
    Third-party libraries
----------------------------------------------------- */
import { PrimengModule } from '@app/lib/primeng.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { NgIdleKeepaliveModule } from '@ng-idle/keepalive';
import { AlerttableService } from '@app/services/alerttable.service';


import { SearchComponent } from '@app/components/search/search/search.component';
import { SearchboxComponent } from '@app/components/search/searchbox/searchbox.component';
import { SearchsuggestionsComponent } from '@app/components/search/searchsuggestions/searchsuggestions.component';
import { GuidesearchComponent } from '@app/components/search/guidedsearch/guidedsearch.component';
import { SharedModule } from '@app/components/shared/shared.module';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from './services/uiconfigs.service';
import { StagingService } from './services/generated/staging.service';

describe('AppComponent', () => {
  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        AppComponent,
        ToolbarComponent,
        SearchComponent,
        SearchboxComponent,
        SearchsuggestionsComponent,
        GuidesearchComponent,
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
        SharedModule,
        NgIdleKeepaliveModule.forRoot()
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        AlerttableService,
        AuthService,
        LogService,
        LogPublishersService,
        Store,
        OverlayContainer,
        MessageService,
        AuthServiceGen,
        UIConfigsService,
        StagingService
      ],
    });
  });

  it('should create the app', () => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app).toBeTruthy();
  });


});
