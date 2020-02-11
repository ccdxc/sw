import { MaterialdesignModule } from '@app/lib/materialdesign.module';

import { ControllerService } from '@app/services/controller.service';
import { EventsService } from '@app/services/events.service';
import { SearchService } from '@app/services/generated/search.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ObjstoreService } from '@app/services/generated/objstore.service';

import { PrimengModule } from '@lib/primeng.module';
import { ConfirmationService } from 'primeng/primeng';
import { WidgetsModule } from 'web-app-framework';
import { SharedModule } from '@app/components/shared/shared.module';
import { AuthService } from '@app/services/auth.service';

import { ApiSampleComponent } from './api-sample.component';
import { ApiCaptureComponent } from './api-capture/api-capture.component';
import { RouterTestingModule } from '@angular/router/testing';
import { ComponentFixture, TestBed, async, tick, fakeAsync, discardPeriodicTasks, flush } from '@angular/core/testing';
import { HttpClientModule } from '@angular/common/http';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { FlexLayoutModule } from '@angular/flex-layout';
import { MatIconRegistry } from '@angular/material';

import { By } from '@angular/platform-browser';

describe('ApiSampleComponent', () => {
  let component: ApiSampleComponent;
  let fixture: ComponentFixture<ApiSampleComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ApiSampleComponent,
        ApiCaptureComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        WidgetsModule,
        FlexLayoutModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        AuthService,
        EventsService,
        SearchService,
        ClusterService,
        MessageService,
        ObjstoreService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ApiSampleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create ApiSampleComponent', () => {
    expect(component).toBeTruthy();
  });

  it('should display live-api capture template, if selected tab index is 1', () => {
    component.selectedIndexChangeEvent(true);
    expect(component.apiLiveTrack).toBeTruthy();
  });

  it('should display api-capture template, if selected tab index is 0', () => {
    component.selectedIndexChangeEvent(false);
    expect(component.apiLiveTrack).toBeFalsy();
  });

  it('should change the template name, on click of tab ', <any>fakeAsync(() => {
    const tabs = fixture.debugElement.queryAll(By.css('.mat-tab-label'));
    expect(tabs.length).toBe(2);
    tabs[1].nativeElement.click();
    fixture.detectChanges();
    tick(20000);
    fixture.detectChanges();
    fixture.whenRenderingDone().then(() => {
      const eventsContainer = fixture.debugElement.query(By.css('.api-samples-capture'));
      const title = eventsContainer.query(By.css('.mat-tab-label'));
      expect(title.nativeElement.textContent).toContain('API Capture');
    });
    discardPeriodicTasks();
    flush();
    fixture.destroy();
  }));
});
