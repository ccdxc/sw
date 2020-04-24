import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { Component } from '@angular/core';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

import { HttpClientTestingModule } from '@angular/common/http/testing';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';

import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';

import { SearchService } from '@app/services/generated/search.service';

import { SharedModule } from '@app/components/shared/shared.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

import { SearchComponent } from '@app/components/search/search/search.component';
import { SearchboxComponent } from '@app/components/search/searchbox/searchbox.component';
import { SearchsuggestionsComponent } from '@app/components/search/searchsuggestions/searchsuggestions.component';
import { SearchresultComponent } from '@app/components/search/searchresult/searchresult.component';
import { GuidesearchComponent } from '@app/components/search/guidedsearch/guidedsearch.component';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';

@Component({
  template: ''
})
class DummyComponent { }

describe('SearchboxComponent', () => {
  let component: SearchboxComponent;
  let fixture: ComponentFixture<SearchboxComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        SearchboxComponent,
        SearchComponent,
        SearchsuggestionsComponent,
        GuidesearchComponent,
        SearchresultComponent,
        DummyComponent,
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        HttpClientTestingModule,
        RouterTestingModule,
        FormsModule,
        SharedModule,
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        LicenseService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService,
        SearchService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(SearchboxComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
