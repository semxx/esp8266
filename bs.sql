/****** Скрипт для команды SelectTopNRows из среды SSMS  ******/
USE [mts_drs_workflow_temp]
GO

SELECT  top 100 a.name, b.[3G_CellName]
,left(A.name, CHARINDEX('_L',A.name)-1) 
,left(b.[3G_CellName], CHARINDEX('_U',b.[3G_CellName])-1)
,b.[csfbPsHoAllowed]
	FROM [mts_drs_workflow_temp].[dbo].[LastOssState_LNCEL] as a with (NOLOCK)
	JOIN [dbo].[4G_3G_Nbrs] as b on a.[name] = b.[4G_source]
 WHERE a.name in ('CELL_77_376_1_L26T')


 -- СПИСОК ВСЕХ АКТИВНЫХ КАНДИДАТОВ НА CSFB ЧЕРЕЗ СИМВОЛ ";" ДЛЯ КАЖДОЙ СОТЫ LTE
select * from (
	select [4G_source], 
		(select cast([3G_CellName] as varchar(max))+ ';' as 'data()' 
			from [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs] t1 with (nolock) 
				where t1.[4G_source] = [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs].[4G_source] and [csfbPsHoAllowed] = 0 
				for xml path('')) [3G_CellName]
			from [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs] with (nolock)
		group by [4G_source]
 ) xxx 

  -- СПИСОК ВСЕХ АКТИВНЫХ КОСАЙТНЫХ КАНДИДАТОВ НА CSFB ЧЕРЕЗ СИМВОЛ ";" ДЛЯ КАЖДОЙ СОТЫ LTE

select * from (
	select [4G_source], 
		(select cast([3G_CellName] as varchar(max))+ ';' as 'data()' 
			from [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs] t1 with (nolock) 
				where t1.[4G_source] = [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs].[4G_source] and [csfbPsHoAllowed] = 0 
--					and left(t1.[4G_source], CHARINDEX('_L',t1.[4G_source])-2) = left(t1.[3G_CellName], CHARINDEX('_U',t1.[3G_CellName])-2)	-- all site nbrs	
					and left(t1.[4G_source], CHARINDEX('_L',t1.[4G_source])-1) = left(t1.[3G_CellName], CHARINDEX('_U',t1.[3G_CellName])-1)	-- co-sector only
				for xml path('')) [3G_CellName]
			from [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs] with (nolock)
		group by [4G_source]
 ) xxx 

  -- ПОДСЧЕТ КОЛИЧЕСТВА ПРОПИСАННЫХ АКТИВНЫХ КАНДИДАТОВ ДЛЯ CSFB 
 select  a.name, count(b.[3G_CellName]) as Active_CSFB_LNRELW
	 from [mts_drs_workflow_temp].[dbo].[LastOssState_LNCEL] as a with (NOLOCK)
	 JOIN [dbo].[4G_3G_Nbrs] as b on a.[name] = b.[4G_source]
	 where b.[csfbPsHoAllowed] = 0
	 group by a.name
 	 order by a.name

 -- ИЛИ
 select a.[4G_source], count(a.[3G_CellName]) as Active_CSFB_LNRELW 
	 from [4G_3G_Nbrs] as a
	 where a.[csfbPsHoAllowed] = 0
	 group by a.[4G_source]
	 order by a.[4G_source]


  -- список на добавление косекторных секторов U2100 и U900 для CSFB, на ячейках LTE где они не прописаны

if object_id('tempdb..#temp4GLNADJW') is not null
    drop table #temp4GLNADJW
if object_id('tempdb..#tmp_lnrelw') is not null
    drop table #tmp_lnrelw
if object_id('tempdb..#tmp_wcel') is not null
    drop table #tmp_wcel
go

select
	 a.[_distName]
    ,d.[name]			as '4G_source'
	,a.cellId1			as '4G_source CI'
	,c.name				as '3G_CellName'
	,a.cellId2			as '3G_target CI'
	,c.UARFCN
	,a.csfbPsHoAllowed
	,b.actCsfbPsHoToUtra
	into #temp4GLNADJW
	from [mts_drs_workflow_temp].[dbo].[LastOssState_LNRELW] as A
	left join 
			[mts_drs_workflow_temp].[dbo].[LastOssState_LNBTS] as B
			on left(A._distname, CHARINDEX('/LNCEL',A._distName)-1) = B._distname 
	left join [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL] as C
			on a.uTargetCid = c.CId
	left join 
			[mts_drs_workflow_temp].[dbo].[LastOssState_LNCEL] as D
			on a.cellId1 = d.eutraCelId

	where d.name is not null
	select * from #temp4GLNADJW

select * into #tmp_lnrelw from (
	select [4G_source], 
		(select cast([3G_CellName] as varchar(max))+ ';' as 'data()' 
			from #temp4GLNADJW t1 with (nolock) 
				where t1.[4G_source] = #temp4GLNADJW.[4G_source]-- and [csfbPsHoAllowed] = 0
--					and left(t1.[4G_source], CHARINDEX('_L',t1.[4G_source])-2) = left(t1.[3G_CellName], CHARINDEX('_U',t1.[3G_CellName])-2)	-- all site nbrs	
					and left(t1.[4G_source], CHARINDEX('_L',t1.[4G_source])-1) = left(t1.[3G_CellName], CHARINDEX('_U',t1.[3G_CellName])-1)	-- co-sector only
				for xml path('')) [3G_CellName]
			from #temp4GLNADJW with (nolock)
		group by [4G_source]
 ) xxx 

select * from #tmp_lnrelw
select name into #tmp_wcel from [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL]
 select *, 
 (select top 1 [name] from #tmp_wcel
 where left(#tmp_wcel.[name], CHARINDEX('_U',[name])-1) = left(#tmp_lnrelw.[4G_source], CHARINDEX('_L',#tmp_lnrelw.[4G_source])-1) and name like '%_U') as UMTS2100_LNRELW, 
  (select top 1 [name] from #tmp_wcel
 where left(#tmp_wcel.[name], CHARINDEX('_U',[name])-1) = left(#tmp_lnrelw.[4G_source], CHARINDEX('_L',#tmp_lnrelw.[4G_source])-1) and name like '%_U900') as UMTS900_LNRELW

from #tmp_lnrelw 
--where [3G_CellName] is null

drop table #temp4GLNADJW
drop table #tmp_wcel
drop table #tmp_lnrelw


-- /////////////////////////// тоже самое, но без временных таблиц ////////////////////////////////////


with cte_lnrelw_cosector as
(
select * from (
	select [4G_source], 
		(select cast([3G_CellName] as varchar(max))+ ';' as 'data()' 
			from [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs] t1 with (nolock) 
				where t1.[4G_source] = [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs].[4G_source] and [csfbPsHoAllowed] = 0
--					and left(t1.[4G_source], CHARINDEX('_L',t1.[4G_source])-2) = left(t1.[3G_CellName], CHARINDEX('_U',t1.[3G_CellName])-2) 	-- all site nbrs	
					and left(t1.[4G_source], CHARINDEX('_L',t1.[4G_source])-1) = left(t1.[3G_CellName], CHARINDEX('_U',t1.[3G_CellName])-1)	-- co-sector only
				for xml path('')) [3G_CellName]
			from [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs] with (nolock) --where tac = 17722
		group by [4G_source]
 ) xxx 
 )
 select *, 
 (select top 1 [name] from [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL] with (nolock) 
 where left([mts_drs_workflow_temp].[dbo].[LastOssState_WCEL].[name], CHARINDEX('_U',[name])-1) = left([4G_source], CHARINDEX('_L',[4G_source])-1) and name like '%_U') as UMTS2100_LNRELW, 
 (select top 1 [name] from [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL] with (nolock) 
 where left([mts_drs_workflow_temp].[dbo].[LastOssState_WCEL].[name], CHARINDEX('_U',[name])-1) = left([4G_source], CHARINDEX('_L',[4G_source])-1) and name like '%_U2') as UMTS2100_2_LNRELW, 
  (select top 1 [name] from [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL] with (nolock)
 where left([mts_drs_workflow_temp].[dbo].[LastOssState_WCEL].[name], CHARINDEX('_U',[name])-1) = left([4G_source], CHARINDEX('_L',[4G_source])-1) and name like '%_U900') as UMTS900_LNRELW
 from cte_lnrelw_cosector
--where [3G_CellName] is null


/* Подсчет количества LNRELW по каждой частоте (активированных/выключенных) */

if object_id('tempdb..#temp4GLNADJW') is not null
    drop table #temp4GLNADJW
go
if object_id('tempdb..#ExistedNbsCountBy_f1f2f3') is not null
	drop table #ExistedNbsCountBy_f1f2f3
select
	 a.[_distName]
    ,d.[name]			as '4G_source'
	,a.cellId1			as '4G_source CI'
	,d.tac
	,c.name				as '3G_CellName'
	,a.cellId2			as '3G_target CI'
	,c.UARFCN
	,c.LAC
	,a.csfbPsHoAllowed
	,b.actCsfbPsHoToUtra
	into #temp4GLNADJW
	from [mts_drs_workflow_temp].[dbo].[LastOssState_LNRELW] as A
	left join 
			[mts_drs_workflow_temp].[dbo].[LastOssState_LNBTS] as B
			on left(A._distname, CHARINDEX('/LNCEL',A._distName)-1) = B._distname 
	left join [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL] as C
			on a.uTargetCid = c.CId
	left join 
			[mts_drs_workflow_temp].[dbo].[LastOssState_LNCEL] as D
			on a.cellId1 = d.eutraCelId

	where d.name is not null
--select * from #temp4GLNADJW where [4G_source] = 'CELL_77_005_1_L18'
 select distinct a.name as CellName
	,a.tac
	,(select  count(*) from #temp4GLNADJW where [UARFCN] = '10762' and #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 0) as [10762]
	,(select  count(*) from #temp4GLNADJW where [UARFCN] = '10762' and #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 1) as [10762(off)]
	,(select  count(*) from #temp4GLNADJW where [UARFCN] = '10738' and #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 0) as [10738]
	,(select  count(*) from #temp4GLNADJW where [UARFCN] = '10738' and #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 1) as [10738(off)]
	,(select  count(*) from #temp4GLNADJW where [UARFCN] = '10713' and #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 0) as [10713]
	,(select  count(*) from #temp4GLNADJW where [UARFCN] = '10713' and #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 1) as [10713(off)]
	,(select  count(*) from #temp4GLNADJW where [UARFCN] = '2987'  and #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 0) as [U900]
	,(select  count(*) from #temp4GLNADJW where [UARFCN] = '2987'  and #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 1) as [U900_off]
	,(select  count(*) from #temp4GLNADJW where  #temp4GLNADJW.[4G_source]=a.[name] and [csfbPsHoAllowed] = 0) as [total_act]
	,(select  count(*) from #temp4GLNADJW where  #temp4GLNADJW.[4G_source]=a.[name]) as [total]
	,a.administrativeState as adm_state
	 into #ExistedNbsCountBy_f1f2f3
	 from [mts_drs_workflow_temp].[dbo].[LastOssState_LNCEL] as a with (NOLOCK)
	 JOIN #temp4GLNADJW as b on a.[name] = b.[4G_source]
  -- where b.[4G_source] = 'CELL_77_005_1_L18'
	 where a.tac = 17722 --and b.[4G_source] = 'CELL_77_150_3_L26T'
 	 order by a.name

select * from #ExistedNbsCountBy_f1f2f3
drop table #temp4GLNADJW


/****** Генерируем косайтных соседей для определенного TAC на добавление и удаление (U3)  ******/

USE [mts_drs_workflow_temp]
go

declare @tac int 

set @tac = 17722

			if object_id('tempdb..#temp3Gcell') is not null
				drop table #temp3Gcell
			select a._distName, a.name, case  len(CHARINDEX('_U',a.[name])) when '1' then 'CELL_77_6666_1_U' when null then 'CELL_77_6666_1_U' else left(a.[name], CHARINDEX('_U',a.[name])-1) end as BTS_ID, a.lac into #temp3Gcell 
			from [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL] a  with (nolock)
			where a.name not like ('%_U3')
			--select * from #temp3Gcell

			if object_id('tempdb..#temp4Gcell') is not null
				drop table #temp4Gcell
			select a._distName, a.name, left(a.[name], CHARINDEX('_L',a.[name])-1) as BTS_ID, a.tac into #temp4Gcell 
			from [mts_drs_workflow_temp].[dbo].[LastOssState_LNCEL] a  with (nolock)
			where a.tac = @tac
			--select * from #temp4Gcell order by [BTS_ID]

			if object_id('tempdb..#4G_3G_CellName') is not null
				drop table #4G_3G_CellName
			select a.[4G_source], a.[3G_CellName] into #4G_3G_CellName 
			from [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs] as a  with (nolock)
			where a.tac = @tac
			--select * from #4G_3G_CellName order by [4G_source], [3G_CellName]

	select b.name as [4G_SRC], a.name as [3G_TGT]
	,(select top 1 'yes' from #4G_3G_CellName as c where c.[4G_source] = b.[name] and c.[3G_CellName] = a.name) as [exist in 3G]
	from #temp3Gcell as a  

	left join #temp4Gcell as b 
								on b.[BTS_ID] = a.[BTS_ID] 
	where a.[BTS_ID]=b.[BTS_ID]

union all -- Добавляем в таблицу записи третьей несущей UMTS, которые необходимо удалить

	select *,'delete' from #4G_3G_CellName a where a.[3G_CellName] like '%U3'

    drop table #temp3Gcell
  drop table #temp4Gcell
drop table #4G_3G_CellName


/****** Проверяем какие соседи есть у косайтной 3G соты  ******/

USE [mts_drs_workflow_temp]
go

declare @tac int 

set @tac = 17722

			if object_id('tempdb..#temp3Gcell') is not null
				drop table #temp3Gcell
select case  len(CHARINDEX('_U',a.[name1])) when '1' then 'CELL_77_6666_1_U' when null then 'CELL_77_6666_1_U' else left(a.[name1], CHARINDEX('_U',a.[name1])-1) end as SecID, a.name1, b.name  into #temp3Gcell from [mts_drs_workflow_temp].[dbo].[LastOssState_ADJI] as a with (nolock)
		     
left join [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL] as b  with (nolock)
									on a.cellId2 = b.CId and a.AdjiLAC = b.LAC
where a.name2 not like '50003-%' and b.name is not null
order by a.name1, b.name

			if object_id('tempdb..#tmpADJI') is not null
				drop table #tmpADJI
--select * from #temp3Gcell
select a.SecID, a.name1, a.name into #tmpADJI from #temp3Gcell a
group by a.SecID, a.name1, a.name


--drop table #tmpADJI
drop table #temp3Gcell

/* создаем таблицу с прописанными объектами */

			if object_id('tempdb..#temp3G') is not null
				drop table #temp3G
			select a._distName, a.name, case  len(CHARINDEX('_U',a.[name])) when '1' then 'CELL_77_6666_1_U' when null then 'CELL_77_6666_1_U' else left(a.[name], CHARINDEX('_U',a.[name])-1) end as BTS_ID, a.lac into #temp3G 
			from [mts_drs_workflow_temp].[dbo].[LastOssState_WCEL] a  with (nolock)
			where a.name not like ('%_U3') 
--			select * from #temp3G order by BTS_ID

			if object_id('tempdb..#temp4G') is not null
				drop table #temp4G
			select a._distName, a.name, left(a.[name], CHARINDEX('_L',a.[name])-1) as BTS_ID, a.tac into #temp4G 
			from [mts_drs_workflow_temp].[dbo].[LastOssState_LNCEL] a  with (nolock)
			where a.tac = @tac
--		    select * from #temp4G

			if object_id('tempdb..#4G_3G_CellName') is not null
				drop table #4G_3G_CellName
			select left(a.[4G_source], CHARINDEX('_L',a.[4G_source])-1) as [L_IDX], a.[4G_source], a.[3G_CellName] into #4G_3G_CellName 
			from [mts_drs_workflow_temp].[dbo].[4G_3G_Nbrs] as a  with (nolock)
			where a.tac = @tac
			--order by [4G_source]
			--select * from #4G_3G_CellName order by L_IDX
drop table #temp3G 
--drop table #temp4G

/* --- выводим таблицу всех прописанных соседей 4G > 3G и добавляем признак существования данного соседа в 3G-соте. */

			if object_id('tempdb..#exist3G_NBs') is not null
				drop table #exist3G_NBs

select a.[4G_Source], a.[3G_CellName]
,(select top 1 'yes' from #tmpADJI as b where a.[L_IDX] = b.[SecID] and a.[3G_CellName] = b.name) as [is exist]
  into #exist3G_NBs
  from #4G_3G_CellName a 
  order by a.[4G_source], a.[3G_CellName]

/* --- выводим таблицу всех потенциальных кандидатов 4G > 3G на основании существущих ко-секторных соседей 3G . */


			if object_id('tempdb..#isExist4G3G') is not null -- во временную таблицу #isExist4G3G помещаем всех прописанных соседей 4G->3G
				drop table #isExist4G3G

  select  left(a.[4G_source], CHARINDEX('_L',a.[4G_source])-1) as L_IDX, a.[4G_source], a.[3G_CellName],a.[is exist] into #isExist4G3G from #exist3G_NBs a where [is exist] = 'yes' order by [4G_source]
 -- select * from  #exist3G_NBs

			if object_id('tempdb..#allCell4G') is not null -- временная таблица с 4G сотами с добавленными индексами для связывания с 3G соседями
				drop table #allCell4G

  select left ([4G_source], CHARINDEX('_L',[4G_source])-1) as L_IDX, [4G_source] into #allCell4G from  #isExist4G3G
  group by [4G_source]
  

-- сливаем предыдущую временнную таблицу с таблицей прописанных сседей #isExist4G3G

			if object_id('tempdb..#add4G3G_nbrs') is not null -- временная таблица с соседями которые необходимо прописать
				drop table #add4G3G_nbrs

  select a.SecID, b.[4G_source] as [4G_CellName], a.name as [3G_Target]
  , c.[is exist] into #add4G3G_nbrs
  from #tmpADJI a 
	left join #allCell4G b on a.SecID = b.[L_IDX]
	left join #isExist4G3G c on b.[4G_source] = c.[4G_source] and a.name = c.[3G_CellName]
  where a.SecID in (select distinct left([4G_source], CHARINDEX('_L',[4G_source])-1) as L_IDX from  #isExist4G3G) 
  group by a.SecID, b.[4G_source], a.name , c.[is exist]
  order by a.SecID, b.[4G_source], a.name

 -- drop table #allCell4G
 -- drop table #isExist4G3G

 			if object_id('tempdb..#del4G3G_nbrs') is not null -- временная таблица с соседями которые необходимо удалить
				drop table #del4G3G_nbrs
				
	select *,'delete' as [delete_nb]  into #del4G3G_nbrs from #4G_3G_CellName a where a.[3G_CellName] not in 
	(select b.[3G_Target] from #add4G3G_nbrs b where a.[4G_source] = b.[4G_CellName])


select * from  #add4G3G_nbrs
union all
select * from  #del4G3G_nbrs	

-- рисуем таблицу с секторами 4G и считаем соседей 3G для каждой несущей, которых можно добавить

			if object_id('tempdb..#NewNbsCountBy_f1f2f3') is not null 
				drop table #NewNbsCountBy_f1f2f3
  select a.[4G_CellName], count(a.[3G_Target]) as [candidates]
  ,(select count(b.[3G_Target]) from #add4G3G_nbrs b where b.[3G_Target] like '%U' and b.[4G_CellName] = a.[4G_CellName] and b.[is exist] is null) as C_U1
  ,(select count(b.[3G_Target]) from #add4G3G_nbrs b where b.[3G_Target] like '%U2' and b.[4G_CellName] = a.[4G_CellName] and b.[is exist] is null) as C_U2
  ,(select count(b.[3G_Target]) from #add4G3G_nbrs b where b.[3G_Target] like '%U3' and b.[4G_CellName] = a.[4G_CellName] and b.[is exist] is null) as C_U3
  --,(select count(c.[3G_CellName]) from #4G_3G_CellName c where c.[4G_source] = a.[4G_CellName]) as DEL_U3
  ,(select count(c.[3G_CellName]) from #del4G3G_nbrs c where c.[3G_CellName] like '%U' and c.[4G_source] = a.[4G_CellName]  ) as D_U1
  ,(select count(c.[3G_CellName]) from #del4G3G_nbrs c where c.[3G_CellName] like '%U2' and c.[4G_source] = a.[4G_CellName] ) as D_U2
  ,(select count(c.[3G_CellName]) from #del4G3G_nbrs c where c.[3G_CellName] like '%U3' and c.[4G_source] = a.[4G_CellName] ) as D_U3 
    into #NewNbsCountBy_f1f2f3
   from #add4G3G_nbrs a where a.[is exist] is null
  group by  a.[4G_CellName]
  order by a.[4G_CellName]

 -- выводим табличку которую посчитали в предыдущем запросе

select b.*,a.total , (b.D_U1+b.D_U2+b.D_U3) as [delete], (a.total - (b.D_U1+b.D_U2+b.D_U3)) as af_del from #ExistedNbsCountBy_f1f2f3 a
left join #NewNbsCountBy_f1f2f3 b on a.[CellName] = b.[4G_CellName]
where b.[4G_CellName] is not null
order by b.[4G_CellName]

-- добавляем статистику для ранжирования кандидатов на добавление

select * from  #add4G3G_nbrs

select top 100 * from [0300TECHSQL01].[NSN_Stat].[Import].[adj_result]

