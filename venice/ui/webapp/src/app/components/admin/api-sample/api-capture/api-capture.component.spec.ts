import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ControllerService } from '@app/services/controller.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { FlexLayoutModule } from '@angular/flex-layout';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { RouterTestingModule } from '@angular/router/testing';

import { TableheaderComponent } from '../../../shared/tableheader/tableheader.component';
import { ApiCaptureComponent, ApiData } from './api-capture.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe.js';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { ConfirmationService } from 'primeng/primeng';
import { AuthService } from '@app/services/auth.service';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { By } from '@angular/platform-browser';

import * as apiStub from '../../../../../assets/apiSample.json';
import { Utility } from '@app/common/Utility';

describe('ApiCaptureComponent', () => {
  let component: ApiCaptureComponent;
  let fixture: ComponentFixture<ApiCaptureComponent>;
  beforeEach(async(() => {
    TestBed.configureTestingModule({
      imports: [
        PrimengModule,
        RouterTestingModule,
        FlexLayoutModule,
        MaterialdesignModule,
        ReactiveFormsModule,
        HttpClientTestingModule,
        FormsModule],
      declarations: [
        ApiCaptureComponent,
        TableheaderComponent,
        PrettyDatePipe
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        MessageService,
        ConfirmationService,
        AuthService,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ApiCaptureComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create ApiCaptureComponent', () => {
    expect(component).toBeTruthy();
  });

  it('should display header as `Live API Capture` on select of live api tab', () => {
    component.isLiveApiTabSelect = true;
    fixture.detectChanges();
    fixture.whenRenderingDone().then(() => {
      const eventsContainer = fixture.debugElement.query(By.css('.tableheader-title'));
      expect(eventsContainer.nativeElement.innerHTML).toContain(' Live API Capture ');
    });
  });

  it('should display header as `API Samples` on select of API capture tab', () => {
    component.isLiveApiTabSelect = false;
    fixture.detectChanges();
    fixture.whenRenderingDone().then(() => {
      const eventsContainer = fixture.debugElement.query(By.css('.tableheader-title'));
      expect(eventsContainer.nativeElement.innerHTML).toContain(' API Samples ');
    });
  });

  it('Should call prepareApiSampleData method on click of API Samples tab ', () => {
    component.prepareApiSampleData();
    expect(component.apiDetails.length).toBeGreaterThan(0);
  });

  it('Should call prepareLiveApiSampleData method on click of live api capture tab ', () => {
    component.isLiveApiTabSelect = true;
    Utility.getInstance().veniceAPISampleMap = {
      '/users/test/IsAuthorized': {
        category: 'auth',
        id: 0,
        kind: 'users',
        request: '',
        response: '',
        url: ''
      }
    };
    component.prepareLiveApiSampleData();
    expect(component.apiDetails.length).toBeGreaterThan(0);
  });

  it('Should convert the value to json string format ', () => {
    const convertedString = component.convertJsonString(null);
    expect(convertedString).toEqual('');
  });

  it('Should add a comment column and return index', () => {
    const returnIndex = component.addCommentsColumn([], 1);
    expect(returnIndex).toBe(1);
  });

});
