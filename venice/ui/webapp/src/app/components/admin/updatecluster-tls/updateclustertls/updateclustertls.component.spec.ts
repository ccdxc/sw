import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import {By} from '@angular/platform-browser';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import {PrimengModule} from '@lib/primeng.module';
import {WidgetsModule} from 'web-app-framework';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared/shared.module';
import { RouterTestingModule } from '@angular/router/testing';

import { Component } from '@angular/core';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { ConfirmationService } from 'primeng/api';
import { AuthService } from '@app/services/auth.service';

import { UpdateclusterTlsComponent } from '../updatecluster-tls.component';
import { UpdateclustertlsComponent } from '../updateclustertls/updateclustertls.component';
import {DebugElement} from '@angular/core';
import {TestingUtility} from '@common/TestingUtility';
import { MatIconRegistry } from '@angular/material';
import { ClusterService } from '@app/services/generated/cluster.service';

@Component({
  template: ''
})
class DummyComponent { }
describe('UpdatecluterTlsComponent', () => {
  let component: UpdateclustertlsComponent;
  let fixture: ComponentFixture<UpdateclustertlsComponent>;
  let de: DebugElement;
  let testingUtility: TestingUtility;

  const keyCrappy = 'hello';
  const certsCrappy = 'world';

  const certs = '-----BEGIN CERTIFICATE-----\nMIIDmjCCAoICCQDjHAsuIK/+CTANBgkqhkiG9w0BAQsFADCBjjELMAkGA1UEBhMC\nVVMxCzAJBgNVBAgMAkNBMREwDwYDVQQHDAhTYW4gSm9zZTEQMA4GA1UECgwHUFMg\nVGVzdDEQMA4GA1UECwwHU1lTVEVTVDEXMBUGA1UEAwwOMTkyLjE2OC43OC4yMTEx\nIjAgBgkqhkiG9w0BCQEWE2FtYnJpc2hAcGVuc2FuZG8uaW8wHhcNMTkwNDAxMjE0\nNzAxWhcNMjAwMzMxMjE0NzAxWjCBjjELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNB\nMREwDwYDVQQHDAhTYW4gSm9zZTEQMA4GA1UECgwHUFMgVGVzdDEQMA4GA1UECwwH\nU1lTVEVTVDEXMBUGA1UEAwwOMTkyLjE2OC43OC4yMTExIjAgBgkqhkiG9w0BCQEW\nE2FtYnJpc2hAcGVuc2FuZG8uaW8wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\nAoIBAQDDpxVdggKdX10gflFonRkhd8Vl+905K8SCbZH1BfZ+Zw9vxVgpdL7KaLXt\n3WvHmlaqEpSqpeZz22aFNoB/prAeTW2M7+GNXlcxLQ4aacudJya/Lj6yliEp7BlL\nTfTXgGemV+3FF9HkneKGGq8g7ExivfdCy+ayuESXTL5yZlnSWQXClKsZ1a/gE2CC\nlVoIlP1b8mAwuKfp83VJ4sJwrpffVQDft43SaZU9w6N0c3/2eijSY4TvGbmECQJj\ngIRT978mnuqNGeym4Ev6IaR30FJ6ssTduZCif8umCNRt4py6rSkCTfRxeLWY8o5l\nJceci4Z4B3R0Jw1JHgnSvuOUu50bAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAFGi\nqDJ+cAxpYEtyG1bsyCjYGJ+QOvfnlVNYPUBM6c/hbUagjrMBd1wAvCAkKM1dGNDA\n/ixCJtbbwDtKrEwhH0WPGSlTsPdTVvdwczV/rN4ih9ndp6b+xDOXQU5U/hkTT8Ye\n3OAP51FSG3wDBgAAc9NXHr2rH3vdsyq9JJSs/g//TU5aVDBiluDAXbTO/O5VI60+\nGIREGVnXrVUIOGvwKF3DpQBXk5Nbw7uzobG0nTSs1i0+sXdAiVL/8lgMMOIfRsZo\nAAr9xicf1prBR8X6Csx5eZEbiiQLhzN6JPdH2NIPUgZ5tqlmpwvEtj9kH4rvT1e6\nDrU+Hw5QR2nMDufMEp0=\n-----END CERTIFICATE-----\n';
  const key = '-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDDpxVdggKdX10g\nflFonRkhd8Vl+905K8SCbZH1BfZ+Zw9vxVgpdL7KaLXt3WvHmlaqEpSqpeZz22aF\nNoB/prAeTW2M7+GNXlcxLQ4aacudJya/Lj6yliEp7BlLTfTXgGemV+3FF9HkneKG\nGq8g7ExivfdCy+ayuESXTL5yZlnSWQXClKsZ1a/gE2CClVoIlP1b8mAwuKfp83VJ\n4sJwrpffVQDft43SaZU9w6N0c3/2eijSY4TvGbmECQJjgIRT978mnuqNGeym4Ev6\nIaR30FJ6ssTduZCif8umCNRt4py6rSkCTfRxeLWY8o5lJceci4Z4B3R0Jw1JHgnS\nvuOUu50bAgMBAAECggEBAK5N4njl5JJzbTIPl1NN8h+g5pnO4u2WEoHoP8o8v7co\n9dWryLlWKsuEsd/yDZ1L4L2vxAaruj2pTxKK1bvjsq+sZ1bSNu2NCLzk2NWrPx0I\nzjowZucPXQhOMNKn5uM+oBjP6SJX8ERubGrwxujb28l0WB6ILUU3aNQqKNkeWhNM\ncRZ5ufbMg174rjXkBmdVw055VgcsKbMB33vND5PCWGdNShlhjQSgYImqOg249ptV\nBVKVC2rIQzAPvTEJ9UkxpmXvaMq8eLC1uj+y5e7baFESqebqAb5ki3inuGgLSmdL\n6hYfTVFttEyJI9pgEt6XiKTzsmSFpuxon/g611T/tTECgYEA55kOaeM35u58BhSr\n1GT3VOpwm5rQtgy2Nlex4FryRQQ89bQFszxVjTKpHPnZ0uNSCe5mrUuJCzsvaRNj\nks+ox5wAjYtCLgXy5K7KALoDggjpugzZUBIiLe+a0l8km2GWAvN1I9Wj5DCASfBT\nxhToy/yB5+wJsrpBDXy12M+VbI8CgYEA2ER3/Dt475eKMbhYJz3AddFO0QbhDp9E\n3Ttk4Ox8TH/r7+L9mcWpp9YFJ+uqji33Q7Jeqb3W03J29QbgT8NvG6MumuiU87Qc\n0sFB0IthQSl4UTeU4Lkq6bpPK7ihNsR+l9jo/N9gTXBZklIGPSGfGfExxUkAmhUC\nD3DCp0ZyZLUCgYEAyz/UmXLyPXWwXHnJrtWJBfnVJ4o/fnU7eAeQMJBAK/ejNBJw\nKjNZ9VeveKGA24XqGpMhgxZWSAgB/Joqec5Klhs/lPjQ9Kj92FnZJ7gXmggObtwV\n3yJ64otz+QnMi5yh1Scp92/nrDwwajNCogT5pQOOUiYhALxamOaIhKTwwtsCgYBp\npok3H5ak9VL9lyD5iqFK7UWjt8iabBQoP6u4dc8DUDripfFd/cIzy8oZBnf7nVj+\nKVU1dxhLk8+pRAKw9LiCt+U8qAKcS5EJJIAScQTB4JB2nMCOR+c+51Vl9nxLVRLC\n+6RWZImXSxt277s8ggBHnBzEVDWHEFxBTsTJExEvQQKBgDtGfD7nc0Yl3GbvRvna\n42QlG0d/wF+5nGFbGBruTLMeN32jihL/PZvyRW0E6cHtl09CDoVV4YE07PL+UPvQ\ndXWZIfnziy+tYLAJ6ZUN88qX/9h6wOTXuHL/+R3oc74ksi9y7vjN6S8+GFGx+DCQ\nzoN0nIoVdOLrwz2ogIhRzDqs\n-----END PRIVATE KEY-----\n';


  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [UpdateclusterTlsComponent, UpdateclustertlsComponent, DummyComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        WidgetsModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule
      ],
      providers: [ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        ClusterService,
        MessageService]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(UpdateclustertlsComponent);
    component = fixture.componentInstance;
    de = fixture.debugElement;
    testingUtility = new TestingUtility(fixture);
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });

  it('should disable and enable upload correspondingly', () => {
    const keyTextArea = de.query(By.css('.updateclustertlsCert-key'));
    const certsTextArea = de.query(By.css('.updateclustertlsCert-certs'));

    // only one input
    testingUtility.setText(keyTextArea, keyCrappy);
    expect(component.isAllInputsValidated()).toBeFalsy();
    // two inputs, should enable upload button
    testingUtility.setText(certsTextArea, certsCrappy);
    expect(component.isAllInputsValidated()).toBeTruthy();

  });

  it('should have correct values after users input', () => {
    const keyTextArea = de.query(By.css('.updateclustertlsCert-key'));
    const certsTextArea = de.query(By.css('.updateclustertlsCert-certs'));
    testingUtility.setText(keyTextArea, keyCrappy);
    testingUtility.setText(certsTextArea, certsCrappy);

    expect(component.newCert.getFormGroupValues().certs).toBe(certsCrappy);
    expect(component.newCert.getFormGroupValues().key).toBe(keyCrappy);
  });
});
