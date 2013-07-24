using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Linq;
using System.Xml.Serialization;
using NSass.Tool.Models;

namespace NSass.Tool
{
	internal class DataStorage
	{
		private const string Filename = "data.xml";

		public DataStorage()
		{
			CreateStorageFileIfNotExist();
		}

		private void CreateStorageFileIfNotExist()
		{
			if (!File.Exists(Filename))
			{
				XDocument xml = new XDocument();
				xml.Add(new XElement("Projects"));
				xml.Save(Filename);
			}
		}

		public void Add(Project project)
		{
			XDocument xml = XDocument.Load(Filename);

			XElement exisitingProject = GetNodeById(xml, project);
			if (exisitingProject == null)
			{
				XElement projectElement = new XElement("Project");

				SerializeProject(project, projectElement);

				xml.Root.Add(projectElement);

				xml.Save(Filename);
			}
		}

		public void Remove(Project project)
		{
			XDocument xml = XDocument.Load(Filename);

			GetNodeById(xml, project).Remove();

			xml.Save(Filename);
		}

		public void Update(Project project)
		{
			XDocument xml = XDocument.Load(Filename);

			XElement exisitingProject = GetNodeById(xml, project);
			exisitingProject.RemoveAll();
			SerializeProject(project, exisitingProject);

			xml.Save(Filename);
		}

		public IEnumerable<Project> GetAll()
		{
			XDocument xml = XDocument.Load(Filename);
			XmlSerializer serializer = new XmlSerializer(typeof(Project));

			return xml.Root.Elements().Select(projectElement => (Project)serializer.Deserialize(projectElement.CreateReader()));
		}

		private static XElement GetNodeById(XDocument xml, Project project)
		{
			return xml.Root.Elements().SingleOrDefault(element => element.Element("Id").Value == project.Id.ToString());
		}

		private static void SerializeProject(Project project, XElement projectElement)
		{
			XmlSerializer serializer = new XmlSerializer(project.GetType());
			using (StringWriter stream = new StringWriter())
			{
				serializer.Serialize(stream, project);
				XDocument serializedProject = XDocument.Parse(stream.ToString());
				foreach (XElement projectProperty in serializedProject.Root.Elements())
				{
					projectElement.Add(projectProperty);
				}
			}
		}
	}
}