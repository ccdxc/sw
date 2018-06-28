import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TableheaderComponent } from './tableheader.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('TableheaderComponent', () => {
  let component: TableheaderComponent;
  let fixture: ComponentFixture<TableheaderComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [TableheaderComponent],
      imports: [
        MaterialdesignModule
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TableheaderComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
