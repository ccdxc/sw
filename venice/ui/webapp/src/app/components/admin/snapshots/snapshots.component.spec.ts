/**-----
 Angular imports
 ------------------*/
import { configureTestSuite } from 'ng-bullet';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { HttpClientModule } from '@angular/common/http';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { EventsService } from '@app/services/events.service';
import { SearchService } from '@app/services/generated/search.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ObjstoreService } from '@app/services/generated/objstore.service';

import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { ConfirmationService } from 'primeng/primeng';
import { WidgetsModule } from 'web-app-framework';
import { SharedModule } from '@app/components/shared/shared.module';
import { AuthService } from '@app/services/auth.service';

import { SnapshotsComponent } from './snapshots.component';


describe('SnapshotsComponent', () => {
  let component: SnapshotsComponent;
  let fixture: ComponentFixture<SnapshotsComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ SnapshotsComponent ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        HttpClientModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        WidgetsModule
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
        UIConfigsService,
        LicenseService,
        AuthService,
        EventsService,
        SearchService,
        ClusterService,
        MessageService,
        ObjstoreService
      ]
    })
    .compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(SnapshotsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

});
