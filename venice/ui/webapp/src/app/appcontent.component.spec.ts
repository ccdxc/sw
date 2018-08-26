/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import { TestBed, async } from '@angular/core/testing';
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
import { CoreModule } from '@app/core';
import { AlertlistModule } from '@app/components/alertlist';
import { LoginModule } from './components/login/login.module';
import { ToolbarComponent } from './widgets/toolbar/toolbar.component';

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
import { SearchresultComponent } from '@app/components/search/searchresult/searchresult.component';
import { GuidesearchComponent } from '@app/components/search/guidedsearch/guidedsearch.component';

import { AppcontentComponent } from '@app/appcontent.component';
import { SearchService } from '@app/services/generated/search.service';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';


describe('AppcontentComponent', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [
        AppcontentComponent,
        ToolbarComponent,
        SearchComponent,
        SearchboxComponent,
        SearchsuggestionsComponent,
        GuidesearchComponent
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
        NoopAnimationsModule
      ],
      providers: [
        ControllerService,
        AlerttableService,
        AuthService,
        LogService,
        LogPublishersService,
        Store,
        SearchService,
        OverlayContainer,
        UIConfigsService
      ],
    });

    TestBed.compileComponents();
  });

  it('should create the app', async(() => {
    const fixture = TestBed.createComponent(AppcontentComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app).toBeTruthy();
  }));


});
